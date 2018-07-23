/*----------------------------------------------------------------------------*/
/* Copyright (c) 2018 FIRST. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
**  This extension reimplements enough of the FRC_Network layer to enable the
**    simulator to communicate with a driver station.  That includes a
**    simple udp layer for communication.
**  The protocol does not appear to be well documented; this implementation
**    is based in part on the Toast ds_comms.cpp by Jaci and in part
**    by the protocol specification given by QDriverStation.
**--------------------------------------------------------------------------*/

#include <sys/types.h>

#include <cstring>
#include <iostream>
#include <wpi/EventLoopRunner.h>
#include "wpi/uv/Udp.h"
#include "wpi/StringRef.h"
#include "wpi/uv/Tcp.h"
#include "wpi/uv/Timer.h"
#include "wpi/uv/util.h"
#include "wpi/raw_ostream.h"

#include <DSCommPacket.h>

#if defined(Win32) || defined(_WIN32)
#pragma comment(lib, "Ws2_32.lib")
#endif

using namespace wpi::uv;


// /*----------------------------------------------------------------------------
// **  TCP thread
// **    This thread only dies at program shut down; it opens a socket
// **  and listens for incoming connections, and then processes data
// **  sent on the socket.
// **--------------------------------------------------------------------------*/
// static void TCPThreadFunc(DSCommPacket* ds) {
//   static const int kTCPPort = 1740;
//   while (true) {
//     int socket = OpenListenSocket(kTCPPort, true);
//     if (socket < 0) {
// #if defined(WIN32) || defined(_WIN32)
//       Sleep(1000);
// #else
//       sleep(1);
// #endif
//       continue;
//     }

//     while (true) {
//       int client = accept(socket, NULL, 0);
//       if (client < 0) {
// #if defined(WIN32) || defined(_WIN32)
//         closesocket(socket);
// #else
//         close(socket);
// #endif
//         break;
//       }

//       uint8_t buf[8192];
//       int len = 0;
//       do {
//         int rc = recv(client, reinterpret_cast<char*>(buf + len),
//                       sizeof(buf) - len, 0);
//         if (rc <= 0) break;
//         len += rc;

//         do {
//           int deduct = ds->DecodeTCP(buf, len);
//           if (deduct <= 0) break;
//           std::memmove(buf, buf + deduct, len - deduct);
//           len -= deduct;
//           if (deduct > 2) ds->SendTCPToHALSim();
//         } while (true);
//       } while (true);

// #if defined(WIN32) || defined(_WIN32)
//       closesocket(client);
//       Sleep(1000);
// #else
//       close(client);
//       sleep(1);
// #endif
//     }
//   }
// }

// /*----------------------------------------------------------------------------
// **  Send a reply packet back to the DS
// **--------------------------------------------------------------------------*/
// static void SendReplyPacket(int socket, struct sockaddr* addr, int addrlen,
//                             int reply_port, DSCommPacket* ds) {
//   static const uint8_t kTagGeneral = 0x01;
//   struct sockaddr_in* in4 = reinterpret_cast<struct sockaddr_in*>(addr);
//   in4->sin_port = htons(reply_port);

//   uint8_t data[8];
//   std::memset(data, 0, sizeof(data));

//   ds->GetIndex(data[0], data[1]);

//   data[2] = kTagGeneral;
//   ds->GetControl(data[3]);
//   ds->GetStatus(data[4]);

//   data[5] = 12;  // Voltage upper
//   data[6] = 0;   // Voltage lower
//   data[7] = 0;   // Request

// #if defined(WIN32) || defined(_WIN32)
//   sendto(socket, reinterpret_cast<const char*>(data), sizeof(data), 0, addr,
//          addrlen);
// #else
//   sendto(socket, data, sizeof(data), 0, addr, addrlen);
// #endif
// }

// /*----------------------------------------------------------------------------
// **  UDP thread
// **    This thread only dies at program shut down; it opens a socket
// **  and listens for incoming connections, and then processes data
// **  sent on the socket.
// **--------------------------------------------------------------------------*/
// static void UDPThreadFunc(DSCommPacket* ds) {
//   static const int kUDPListenPort = 1110;
//   static const int kUDPReplyPort = 1150;
//   while (true) {
//     int socket = OpenListenSocket(kUDPListenPort, false);
//     if (socket < 0) {
// #if defined(WIN32) || defined(_WIN32)
//       Sleep(1000);
// #else
//       sleep(1);
// #endif
//       continue;
//     }

//     do {
//       uint8_t buf[1024];
//       struct sockaddr addr;
// #if defined(Win32) || defined(_WIN32)
//       int addrlen = sizeof(addr);
//       int rc = recvfrom(socket, reinterpret_cast<char*>(buf), sizeof(buf), 0,
//                         &addr, &addrlen);
// #else
//       socklen_t addrlen = sizeof(addr);
//       ssize_t rc = recvfrom(socket, buf, sizeof(buf), 0, &addr, &addrlen);
// #endif

//       if (rc > 0) {
//         ds->DecodeUDP(buf, rc);
//         SendReplyPacket(socket, &addr, addrlen, kUDPReplyPort, ds);
//         ds->SendUDPToHALSim();
//       } else {
//         break;
//       }
//     } while (true);
//   }
// }

namespace {
  struct DataStore {
  wpi::SmallVector<char, 128> m_frame;
  size_t m_frameSize = std::numeric_limits<size_t>::max();
  halsim::DSCommPacket* dsPacket;
};
}

static void HandleTcpDataStream(Buffer& buf, size_t size, DataStore& store) {
  wpi::StringRef data{buf.base, size};
  while (!data.empty()) {
    if (store.m_frameSize != std::numeric_limits<size_t>::max()) {
      if (store.m_frame.size() < 2u) {
          size_t toCopy = std::min(2u - store.m_frame.size(), data.size());
          store.m_frame.append(data.bytes_begin(), data.bytes_begin() + toCopy);
          data = data.drop_front(toCopy);
          if (store.m_frame.size() < 2u) return; // need more data
      }
      store.m_frameSize = (static_cast<uint16_t>(store.m_frame[0]) << 8) | static_cast<uint16_t>(store.m_frame[1]);
    }
    if (store.m_frameSize != 0) {
      size_t need = store.m_frameSize - (store.m_frame.size() - 2);
      size_t toCopy = std::min(need, data.size());
      store.m_frame.append(data.bytes_begin(), data.bytes_begin() + toCopy);
      data = data.drop_front(toCopy);
      need -= toCopy;
      if (need == 0) {
        auto ds = store.dsPacket;
        ds->DecodeTCP(reinterpret_cast<uint8_t*>(store.m_frame.data()) , store.m_frame.size());
        ds->SendTCPToHALSim();
        store.m_frame.clear();
        store.m_frameSize = std::numeric_limits<size_t>::max();
      }
    }
  }
}

static void SetupTcp(wpi::uv::Loop& loop) {
  auto tcp = Tcp::Create(loop);
  auto tcpWaitTimer = Timer::Create(loop);

  auto recStore = std::make_shared<DataStore>();

  tcp->SetData(recStore);

  tcp->data.connect([t = tcp.get()](Buffer& buf, size_t len){
    wpi::outs() << "Received Data\n";
    wpi::outs().flush();
      HandleTcpDataStream(buf, len, *t->GetData<DataStore>());
      // Data is here
    });

  tcp->error.connect([t = tcpWaitTimer.get(), s = tcp.get()](Error err){
        wpi::outs() << err.str() << "\n";
        wpi::outs().flush();
    if (err.code() == UV_ECONNREFUSED) {
      t->Start(Timer::Time{1000});
      return;
    }
    s->Reuse([t]{
      t->Start(Timer::Time{1000});
    });
  });
  tcpWaitTimer->timeout.connect([s = tcp, t = tcpWaitTimer.get()]{
    t->Stop();
    s->Connect("127.0.0.1", 1740, [si = s.get()]{
          wpi::outs() << "Connected\n";
    wpi::outs().flush();
      si->StartRead();
    });
  });

  tcp->end.connect([t = tcpWaitTimer.get(), s = tcp.get()]{
    s->Reuse([t](){
      t->Start(Timer::Time{5000});
    });
  });
  tcp->Connect("127.0.0.1", 1740, [t = tcp.get()]{
        wpi::outs() << "Connected\n";
    wpi::outs().flush();
    t->StartRead();
  });

}

/*----------------------------------------------------------------------------
**  Send a reply packet back to the DS
**--------------------------------------------------------------------------*/
static void SetupReplyPacket(halsim::DSCommPacket* ds) {
  static const uint8_t kTagGeneral = 0x01;

  uint8_t* data = reinterpret_cast<uint8_t*>(ds->GetSendBuffer().base);

  ds->GetIndex(data[0], data[1]);

  data[2] = kTagGeneral;
  ds->GetControl(data[3]);
  ds->GetStatus(data[4]);

  data[5] = 12;  // Voltage upper
  data[6] = 0;   // Voltage lower
  data[7] = 0;   // Request
}

static std::unique_ptr<Buffer> singleByte;

static void SetupUdp(wpi::uv::Loop& loop) {
  auto udp = wpi::uv::Udp::Create(loop);
  udp->Bind("0.0.0.0", 1110);


  // Simulation mode packet
  auto simLoopTimer = Timer::Create(loop);
  struct sockaddr_in simAddr;
  NameToAddr("127.0.0.1", 1135, &simAddr);

  simLoopTimer->timeout.connect([udpLocal = udp.get(), simAddr]{
    udpLocal->Send(simAddr, wpi::ArrayRef<Buffer>{singleByte.get(), 1}, [](auto& buf, Error err){
    });
  });
  simLoopTimer->Start(Timer::Time{100}, Timer::Time{100});


  // UDP Receive then send
  udp->received.connect([udpLocal = udp.get()](Buffer& buf, size_t len, const sockaddr& recSock, unsigned int port){
    auto ds = udpLocal->GetLoop()->GetData<halsim::DSCommPacket>();
    ds->DecodeUDP(reinterpret_cast<uint8_t*>(buf.base), len);
    SetupReplyPacket(ds.get());

    struct sockaddr_in outAddr;
    //NameToAddr("127.0.0.1", 1150, &outAddr);

    std::memcpy(&outAddr, &recSock, sizeof(sockaddr_in));
    outAddr.sin_family = PF_INET;
    outAddr.sin_port = htons(1150);
    udpLocal->Send(outAddr, wpi::ArrayRef<Buffer>{&ds->GetSendBuffer(), 1}, [](auto& buf, Error err){
    });
    ds->SendUDPToHALSim();

  });

  udp->StartRecv();
}

static void SetupEventLoop(wpi::uv::Loop& loop) {
  auto loopData = std::make_shared<halsim::DSCommPacket>();
  loop.SetData(loopData);
  SetupUdp(loop);
  SetupTcp(loop);
}

static std::unique_ptr<wpi::EventLoopRunner> eventLoopRunner;

/*----------------------------------------------------------------------------
** Main entry point.  We will start listen threads going, processing
**  against our driver station packet
**--------------------------------------------------------------------------*/
extern "C" {
#if defined(WIN32) || defined(_WIN32)
__declspec(dllexport)
#endif
    int HALSIM_InitExtension(void) {
  static bool once = false;


  if (once) {
    std::cerr << "Error: cannot invoke HALSIM_InitExtension twice."
              << std::endl;
    return -1;
  }
  once = true;

  singleByte = std::make_unique<Buffer>("0");

  std::cout << "DriverStationSocket Initializing." << std::endl;

  eventLoopRunner = std::make_unique<wpi::EventLoopRunner>();

  eventLoopRunner->ExecAsync(SetupEventLoop);

  std::cout << "DriverStationSocket Initialized!" << std::endl;
  return 0;
}
}  // extern "C"

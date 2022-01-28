#include "RioLog.h"
#include "wpi/Logger.h"
#include "wpi/uv/Tcp.h"

using namespace rl;
namespace uv = wpi::uv;

void RioLog::AsyncConnect(const AsyncMessage& msg) {
  if (msg.address >= 0) {
    m_directAddress = msg.address;
  }

  if (msg.teamNumber >= 0) {
    m_teamNumber = msg.teamNumber;
  }
  std::vector<std::pair<std::string, unsigned int>> addresses;
  if (m_directAddress >= 0) {
    struct in_addr in;
    in.s_addr = m_directAddress;
    addresses.emplace_back(std::make_pair(inet_ntoa(in), m_port));
  }
  m_connect->SetServers(addresses);
}

RioLog::RioLog(int port) : m_port{port} {
  wpi::Logger logger;
  m_loop.ExecSync([&](uv::Loop& loop) {
    m_connect = wpi::ParallelTcpConnector::Create(
        loop, uv::Timer::Time{2000}, logger, [&](uv::Tcp& tcp) {
          tcp.data.connect(
              [&](uv::Buffer& buf, size_t len) { HandleRead(buf, len); });
          tcp.StartRead();
          m_connect->Succeeded(tcp);
          {
            std::scoped_lock lock{m_messagesMutex};
            m_messages.emplace_back(Message{42, 56});
          }
          tcp.end.connect([&] {
            tcp.Close();
            m_connect->Disconnected();
          });
          tcp.error.connect([&](uv::Error) { m_connect->Disconnected(); });
        });

    m_asyncTrigger = wpi::uv::Async<AsyncMessage>::Create(loop);
    m_asyncTrigger->wakeup.connect(
        [&](AsyncMessage msg) { AsyncConnect(msg); });
  });

  //   });
}

void RioLog::SetDsIpAddress(int64_t address) {
  AsyncMessage msg;
  msg.address = address;
  m_asyncTrigger->Send(msg);
}

void RioLog::HandleRead(uv::Buffer& buf, size_t len) {
  wpi::span<uint8_t> data{reinterpret_cast<uint8_t*>(buf.base), len};
  while (!data.empty()) {
    if (m_frameSize == (std::numeric_limits<size_t>::max)()) {
      if (m_frameData.size() < 2u) {
        size_t toCopy = (std::min)(2u - m_frameData.size(), data.size());
        m_frameData.append(data.begin(), data.begin() + toCopy);
        data = data.subspan(toCopy);
        if (m_frameData.size() < 2u)
          return;  // need more data
      }
      m_frameSize = (static_cast<uint16_t>(m_frameData[0]) << 8) |
                    static_cast<uint16_t>(m_frameData[1]);
    }
    if (m_frameSize != (std::numeric_limits<size_t>::max)()) {
      size_t need = m_frameSize - (m_frameData.size() - 2);
      size_t toCopy = (std::min)(need, data.size());
      m_frameData.append(data.begin(), data.begin() + toCopy);
      data = data.subspan(toCopy);
      need -= toCopy;
      if (need == 0) {
        HandleData(m_frameData);
        m_frameData.clear();
        m_frameSize = (std::numeric_limits<size_t>::max)();
      }
    }
  }
}

static constexpr uint16_t ReadU16BE(wpi::span<uint8_t> data, size_t offset) {
  uint16_t read = data[offset] << 8 | data[offset + 1];
  return read;
}

std::vector<Message> RioLog::GetMessages() {
  std::scoped_lock lock{m_messagesMutex};
  if (m_messages.empty()) {
    return {};
  }
  std::vector<Message> ret;
  m_messages.swap(ret);
  return ret;
}

void RioLog::HandleData(wpi::span<uint8_t> data) {
  size_t count = 0;
  size_t len = 0;
  do {
    len = ReadU16BE(data, count);
    count += 2;
  } while (len == 0);

  uint8_t tag = data[count];
  count++;

  auto outputBuffer = data.subspan(count);
  if (tag == 11) {
    std::scoped_lock lock{m_messagesMutex};
    m_messages.emplace_back(Message{tag, outputBuffer.size()});
  } else if (tag == 12) {
    std::scoped_lock lock{m_messagesMutex};
    m_messages.emplace_back(Message{tag, outputBuffer.size()});
  }
}

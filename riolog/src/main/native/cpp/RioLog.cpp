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

static constexpr uint16_t ReadU16BE(wpi::span<uint8_t>& data) {
  uint16_t read = data[0] << 8 | data[1];
  data = data.subspan(2);
  return read;
}

static constexpr int16_t Read16BE(wpi::span<uint8_t>& data) {
  uint16_t read = data[0] << 8 | data[1];
  data = data.subspan(2);
  return read;
}

static constexpr int32_t Read32BE(wpi::span<uint8_t>& data) {
  uint32_t read = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
  data = data.subspan(4);
  return read;
}

static float ReadFloatBE(wpi::span<uint8_t>& data) {
  char buf[4]{};
  buf[3] = data[0];
  buf[2] = data[1];
  buf[1] = data[2];
  buf[0] = data[3];
  data = data.subspan(4);
  return *reinterpret_cast<float*>(buf);
}

static std::string ReadSizedString(wpi::span<uint8_t>& data) {
  size_t size = ReadU16BE(data);
  std::string ret =
      std::string{reinterpret_cast<const char*>(data.data()), size};
  data = data.subspan(size);
  return ret;
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
  uint16_t size = ReadU16BE(data);
  uint8_t tag = data[0];
  data = data.subspan(1);
  Message msg;
  msg.timestamp = ReadFloatBE(data);
  msg.sequenceNumber = ReadU16BE(data);
  if (tag == 11) {
    // Error
    Error error;
    error.numOccurance = ReadU16BE(data);
    error.errorCode = Read32BE(data);
    error.flags = data[0];
    data = data.subspan(1);
    error.details = ReadSizedString(data);
    error.location = ReadSizedString(data);
    error.callStack = ReadSizedString(data);
    msg.value = std::move(error);
  } else if (tag == 12) {
    // Print
    Print print;
    if (!data.empty()) {
      print.line = std::string{reinterpret_cast<const char*>(data.data()), data.size()};
    }
    msg.value = std::move(print);
  } else {
    // Invalid
    return;
  }
  std::scoped_lock lock{m_messagesMutex};
  m_messages.emplace_back(std::move(msg));
}

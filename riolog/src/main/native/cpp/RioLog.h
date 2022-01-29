#pragma once

#include <wpi/EventLoopRunner.h>
#include <wpi/ParallelTcpConnector.h>
#include <wpi/uv/AsyncFunction.h>
#include <wpi/uv/Async.h>
#include <vector>
#include <wpi/mutex.h>
#include <wpi/SmallVector.h>
#include <variant>

namespace rl {

struct Print {
  std::string line;
};

struct Error {
  int32_t errorCode;
  uint16_t numOccurance;
  uint8_t flags;
  std::string details;
  std::string location;
  std::string callStack;

  constexpr bool IsWarning() const {
    return (flags & 1) == 0;
  }
};

struct Message {
  float timestamp;
  uint16_t sequenceNumber;
  std::variant<Print, Error> value;
};

class RioLog {
 public:
  explicit RioLog(int port = 1741);
  void SetTeamNumber(int teamNumber);
  void SetDsIpAddress(int64_t address);

  std::vector<Message> GetMessages();

 private:
  struct AsyncMessage {
    int32_t teamNumber{-1};
    int64_t address{-1};
  };
  void AsyncConnect(const AsyncMessage& msg);
  void HandleRead(wpi::uv::Buffer& buf, size_t len);
  void HandleData(wpi::span<uint8_t> data);
  wpi::EventLoopRunner m_loop;
  std::shared_ptr<wpi::ParallelTcpConnector> m_connect;
  std::shared_ptr<wpi::uv::Async<AsyncMessage>> m_asyncTrigger;
  std::vector<Message> m_messages;
  wpi::mutex m_messagesMutex;
  int m_teamNumber{-1};
  int64_t m_directAddress{-1};
  int m_port;

  wpi::SmallVector<uint8_t, 128> m_frameData;
  size_t m_frameSize = (std::numeric_limits<size_t>::max)();
};
}  // namespace rl

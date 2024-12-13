// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

#include "hal/CAN.h"

#include "CANLowLevel.h"

#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cstdio>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include <wpi/DenseMap.h>
#include <wpi/mutex.h>
#include <wpi/timestamp.h>

#include "hal/Errors.h"
#include "hal/Threads.h"
#include "wpinet/EventLoopRunner.h"
#include "wpinet/uv/Poll.h"
#include "wpinet/uv/Timer.h"

#include "hal/handles/UnlimitedHandleResource.h"
#include "hal/handles/HandlesInternal.h"

#define NUM_CAN_BUSES 1

namespace {

static constexpr uint32_t MatchingBitMask = CAN_EFF_MASK | CAN_RTR_FLAG;

// Everything but the API Id
static constexpr uint32_t ReceiveCheckMask = 0x1FFF003F;
static constexpr uint32_t ToArbId(HAL_CANManufacturer manufacturer,
                                  HAL_CANDeviceType deviceType, uint16_t apiId,
                                  uint8_t deviceId) {
  uint32_t createdId = 0;
  createdId |= (static_cast<uint32_t>(deviceType) & 0x1F) << 24;
  createdId |= (static_cast<uint32_t>(manufacturer) & 0xFF) << 16;
  createdId |= (static_cast<uint32_t>(apiId) & 0x3FF) << 6;
  createdId |= (deviceId & 0x3F);
  return createdId;
}

static_assert(CAN_RTR_FLAG == HAL_CAN_IS_FRAME_REMOTE);
static_assert(sizeof(canfd_frame::data) == sizeof(hal::CanFrame::data));

struct SocketCanState {
  wpi::EventLoopRunner readLoopRunner;
  wpi::EventLoopRunner writeLoopRunner;
  wpi::mutex writeMutex[NUM_CAN_BUSES];
  // TODO use a separate socket for periodic writes to reduce contention
  int socketHandle[NUM_CAN_BUSES];
  // ms to count/timer map
  wpi::DenseMap<uint16_t, std::pair<size_t, std::weak_ptr<wpi::uv::Timer>>>
      timers;
  // ms to bus mask/packet
  wpi::DenseMap<uint16_t, std::array<std::optional<canfd_frame>, NUM_CAN_BUSES>>
      timedFrames;
  // packet to time
  wpi::DenseMap<uint32_t, std::array<uint16_t, NUM_CAN_BUSES>> packetToTime;

  wpi::SmallDenseMap<uint32_t,
                     wpi::SmallVector<std::weak_ptr<hal::CanStream>, 4>>
      receiveStreams[NUM_CAN_BUSES];

  void HandleReceivedFrame(uint8_t busId, const canfd_frame& frame,
                           uint64_t timestamp);

  bool InitializeBuses();

  void TimerCallback(uint16_t time);

  void RemovePeriodic(uint8_t busMask, uint32_t messageId);
  void AddPeriodic(wpi::uv::Loop& loop, uint8_t busMask, uint16_t time,
                   const canfd_frame& frame);
};

}  // namespace

static SocketCanState* canState;

namespace hal::init {
void InitializeCAN() {
  canState = new SocketCanState{};
}
}  // namespace hal::init

bool SocketCanState::InitializeBuses() {
  bool success = true;

  writeLoopRunner.ExecSync([this](wpi::uv::Loop&) {
    int32_t status = 0;
    HAL_SetCurrentThreadPriority(true, 30, &status);
    if (status != 0) {
      std::printf("Failed to set CAN write thread priority\n");
    }
  });

  readLoopRunner.ExecSync([this, &success](wpi::uv::Loop& loop) {
    int32_t status = 0;
    HAL_SetCurrentThreadPriority(true, 50, &status);
    if (status != 0) {
      std::printf("Failed to set CAN read thread priority\n");
    }

    for (int i = 0; i < NUM_CAN_BUSES; i++) {
      std::scoped_lock lock{writeMutex[i]};
      socketHandle[i] =
          socket(PF_CAN, SOCK_RAW | SOCK_NONBLOCK | SOCK_CLOEXEC, CAN_RAW);
      if (socketHandle[i] == -1) {
        success = false;
        return;
      }

      ifreq ifr;
      std::snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "can%d", i);

      if (ioctl(socketHandle[i], SIOCGIFINDEX, &ifr) == -1) {
        success = false;
        return;
      }

      sockaddr_can addr;
      std::memset(&addr, 0, sizeof(addr));
      addr.can_family = AF_CAN;
      addr.can_ifindex = ifr.ifr_ifindex;

      if (bind(socketHandle[i], reinterpret_cast<const sockaddr*>(&addr),
               sizeof(addr)) == -1) {
        success = false;
        return;
      }

      std::printf("Successfully bound to can interface %d\n", i);

      auto poll = wpi::uv::Poll::Create(loop, socketHandle[i]);
      if (!poll) {
        success = false;
        return;
      }

      poll->pollEvent.connect(
          [this, fd = socketHandle[i], canIndex = i](int mask) {
            if (mask & UV_READABLE) {
              canfd_frame frame;
              int rVal = read(fd, &frame, sizeof(frame));
              auto now = wpi::Now();
              if (rVal != CAN_MTU && rVal != CANFD_MTU) {
                // TODO(thadhouse) error handling
                return;
              }
              if (frame.can_id & CAN_ERR_FLAG) {
                // Do nothing if this is an error frame
                return;
              }

              if (rVal == CANFD_MTU) {
                frame.flags = CANFD_FDF;
              }

              HandleReceivedFrame(canIndex, frame, now);
            }
          });

      poll->Start(UV_READABLE);
    }
  });
  return success;
}

void SocketCanState::TimerCallback(uint16_t time) {
  auto& busFrames = timedFrames[time];
  for (size_t i = 0; i < busFrames.size(); i++) {
    const auto& frame = busFrames[i];
    if (!frame.has_value()) {
      continue;
    }
    std::scoped_lock lock{writeMutex[i]};
    int mtu = (frame->flags & CANFD_FDF) ? CANFD_MTU : CAN_MTU;
    send(canState->socketHandle[i], &*frame, mtu, 0);
  }
}

void SocketCanState::RemovePeriodic(uint8_t busId, uint32_t messageId) {
  // Find time, and remove from map
  auto& time = packetToTime[messageId][busId];
  auto storedTime = time;
  time = 0;

  // Its already been removed
  if (storedTime == 0) {
    return;
  }

  // Reset frame
  timedFrames[storedTime][busId].reset();

  auto& timer = timers[storedTime];
  // Stop the timer
  timer.first--;
  if (timer.first == 0) {
    if (auto l = timer.second.lock()) {
      l->Stop();
    }
  }
}

void SocketCanState::AddPeriodic(wpi::uv::Loop& loop, uint8_t busId,
                                 uint16_t time, const canfd_frame& frame) {
  packetToTime[frame.can_id][busId] = time;
  timedFrames[time][busId] = frame;
  auto& timer = timers[time];
  timer.first++;
  if (timer.first == 1) {
    auto newTimer = wpi::uv::Timer::Create(loop);
    newTimer->timeout.connect([this, time] { TimerCallback(time); });
    newTimer->Start(wpi::uv::Timer::Time{time}, wpi::uv::Timer::Time{time});
  }
}

void SocketCanState::HandleReceivedFrame(uint8_t busId,
                                         const canfd_frame& frame,
                                         uint64_t timestamp) {
  uint32_t filterId = frame.can_id & ReceiveCheckMask;
  uint16_t apiId = (frame.can_id >> 6) & 0x3FF;
  for (auto&& weakStream : receiveStreams[busId][filterId]) {
    if (auto stream = weakStream.lock()) {
      stream->InsertNewFrame(apiId, frame, timestamp);
    }
  }
}

namespace hal {
bool InitializeCanBuses() {
  return canState->InitializeBuses();
}
}  // namespace hal

namespace {}  // namespace

extern "C" {

void HAL_CAN_SendMessage(uint32_t messageID, const uint8_t* data,
                         uint8_t dataSize, int32_t periodMs, int32_t* status) {
  *status = HAL_HANDLE_ERROR;
  return;
}
void HAL_CAN_ReceiveMessage(uint32_t* messageID, uint32_t messageIDMask,
                            uint8_t* data, uint8_t* dataSize,
                            uint32_t* timeStamp, int32_t* status) {
  *status = HAL_ERR_CANSessionMux_MessageNotFound;
  return;
}
void HAL_CAN_OpenStreamSession(uint32_t* sessionHandle, uint32_t messageID,
                               uint32_t messageIDMask, uint32_t maxMessages,
                               int32_t* status) {
  *sessionHandle = 0;
  *status = HAL_HANDLE_ERROR;
  return;
}
void HAL_CAN_CloseStreamSession(uint32_t sessionHandle) {}
void HAL_CAN_ReadStreamSession(uint32_t sessionHandle,
                               struct HAL_CANStreamMessage* messages,
                               uint32_t messagesToRead, uint32_t* messagesRead,
                               int32_t* status) {
  *status = HAL_HANDLE_ERROR;
  return;
}
void HAL_CAN_GetCANStatus(float* percentBusUtilization, uint32_t* busOffCount,
                          uint32_t* txFullCount, uint32_t* receiveErrorCount,
                          uint32_t* transmitErrorCount, int32_t* status) {
  *status = HAL_HANDLE_ERROR;
  return;
}
}  // extern "C"

using namespace hal;

CanStream::~CanStream() = default;

int32_t CanStream::WriteFrame(const CanFrame& frame, uint16_t apiId,
                              int32_t periodMs) {
  auto arbId = ToArbId(manufacturer, deviceType, apiId, deviceId);
  return WriteDirectFrame(frame, busId, arbId, periodMs);
}

// TODO(thadhouse) What is the netcomm behavior here if a
// periodic frame is sent a new packet with no repeat set.
int32_t CanStream::WriteDirectFrame(const CanFrame& frame, uint8_t busId,
                                    uint32_t arbId, int32_t periodMs) {
  if (busId >= NUM_CAN_BUSES) {
    return PARAMETER_OUT_OF_RANGE;
  }

  // Require all extra bits to not be set
  if ((arbId & ~CAN_EFF_MASK) != 0) {
    return PARAMETER_OUT_OF_RANGE;
  }

  if (periodMs == HAL_CAN_SEND_PERIOD_STOP_REPEATING) {
    canState->writeLoopRunner.ExecSync([busId, arbId](wpi::uv::Loop&) {
      canState->RemovePeriodic(busId, arbId);
    });

    return 0;
  }

  if (frame.isFd && frame.length > CANFD_MAX_DLEN) {
    return PARAMETER_OUT_OF_RANGE;
  } else if (!frame.isFd && frame.length > CAN_MAX_DLEN) {
    return PARAMETER_OUT_OF_RANGE;
  }

  bool isFd = frame.isFd;

  canfd_frame osFrame;
  std::memset(&osFrame, 0, sizeof(osFrame));
  osFrame.can_id = arbId | CAN_EFF_FLAG;
  osFrame.flags = isFd ? CANFD_FDF | CANFD_BRS : 0;
  osFrame.len =
      (std::min)(frame.length, static_cast<uint8_t>(sizeof(osFrame.data)));
  std::memcpy(osFrame.data, frame.data, osFrame.len);

  if (frame.isRtr) {
    osFrame.can_id |= CAN_RTR_FLAG;
  }

  int mtu = isFd ? CANFD_MTU : CAN_MTU;
  {
    std::scoped_lock lock{canState->writeMutex[busId]};
    int result = send(canState->socketHandle[busId], &osFrame, mtu, 0);
    if (result != mtu) {
      // TODO(thadhouse) better error
      return HAL_ERR_CANSessionMux_InvalidBuffer;
    }
  }

  if (periodMs > 0) {
    canState->writeLoopRunner.ExecAsync(
        [busId, periodMs, osFrame](wpi::uv::Loop& loop) {
          canState->AddPeriodic(loop, busId, periodMs, osFrame);
        });
  }

  return 0;
}

std::shared_ptr<MappedCanStream> CanStream::ConstructMapped(
    uint8_t busId, HAL_CANManufacturer manufacturer,
    HAL_CANDeviceType deviceType, uint8_t deviceId) {
  if (busId >= NUM_CAN_BUSES) {
    return nullptr;
  }
  auto ptr = std::make_shared<MappedCanStream>(
      private_init{}, busId, manufacturer, deviceType, deviceId);
  std::weak_ptr<CanStream> weak = ptr;
  uint32_t arbIdFilter = ToArbId(manufacturer, deviceType, 0, deviceId);
  canState->readLoopRunner.ExecSync(
      [busId, arbIdFilter, weak = std::move(weak)](wpi::uv::Loop&) mutable {
        auto& streams = canState->receiveStreams[busId][arbIdFilter];

        for (auto&& i : streams) {
          if (i.expired()) {
            i = std::move(weak);
            return;
          }
        }
        streams.emplace_back(std::move(weak));
      });
  return ptr;
}

std::shared_ptr<AllCanStream> CanStream::ConstructAll(
    uint8_t busId, HAL_CANManufacturer manufacturer,
    HAL_CANDeviceType deviceType, uint8_t deviceId) {
  if (busId >= NUM_CAN_BUSES) {
    return nullptr;
  }
  auto ptr = std::make_shared<AllCanStream>(private_init{}, busId, manufacturer,
                                            deviceType, deviceId);
  std::weak_ptr<CanStream> weak = ptr;
  uint32_t arbIdFilter = ToArbId(manufacturer, deviceType, 0, deviceId);
  canState->readLoopRunner.ExecSync(
      [busId, arbIdFilter, weak = std::move(weak)](wpi::uv::Loop&) mutable {
        auto& streams = canState->receiveStreams[busId][arbIdFilter];

        for (auto&& i : streams) {
          if (i.expired()) {
            i = std::move(weak);
            return;
          }
        }
        streams.emplace_back(std::move(weak));
      });
  return ptr;
}

static void OsToHalFrame(const canfd_frame& osFrame, hal::CanFrame* frame) {
  frame->isFd = (osFrame.flags & CANFD_FDF) != 0;
  frame->isRtr = (osFrame.can_id & CAN_RTR_FLAG) != 0;
  frame->length =
      (std::min)(osFrame.len, static_cast<uint8_t>(sizeof(osFrame.data)));
  std::memcpy(frame->data, osFrame.data, frame->length);
}

void MappedCanStream::InsertNewFrame(uint16_t apiId, const canfd_frame& frame,
                                     uint64_t timestamp) {
  if (apiId > 0x3FF) {
    printf("Bugbug invalid can id\n");
    return;
  }
  // Don't insert RTR frames into the map
  if (frame.can_id & CAN_RTR_FLAG) {
    return;
  }
  std::scoped_lock lock{dataMutex};
  auto& msg = frames[apiId];
  OsToHalFrame(frame, &msg.frame);
  msg.timestamp = timestamp;
  msg.hasBeenRead = false;
}

std::optional<ReceivedCanFrame> MappedCanStream::ReadFrame(uint16_t apiId) {
  if (apiId > 0x3FF) {
    return {};
  }
  std::scoped_lock lock{dataMutex};
  auto& msg = frames[apiId & 0x3FF];
  ReceivedCanFrame copy = msg;
  msg.hasBeenRead = true;
  return copy;
}

void AllCanStream::InsertNewFrame(uint16_t apiId, const canfd_frame& frame,
                                  uint64_t timestamp) {
  if (apiId > 0x3FF) {
    printf("Bugbug invalid can id\n");
    return;
  }
  std::scoped_lock lock{dataMutex};
  auto& msg = frames.emplace_back();
  OsToHalFrame(frame, &msg.frame);
  msg.timestamp = timestamp;
  msg.hasBeenRead = false;
  newDataEvent.Set();
}

std::vector<ReceivedCanFrame> AllCanStream::GetFrames() {
  std::scoped_lock lock{dataMutex};
  newDataEvent.Reset();
  if (frames.empty()) {
    return {};
  }
  std::vector<ReceivedCanFrame> ret;
  frames.swap(ret);
  return ret;
}

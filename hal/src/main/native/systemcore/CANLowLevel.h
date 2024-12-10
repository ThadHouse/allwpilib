#pragma once

#include <wpi/Synchronization.h>

#include <memory>
#include <hal/CANAPI.h>
#include <wpi/mutex.h>
#include <wpi/DenseMap.h>
#include <optional>

#include <linux/can.h>

namespace hal {

struct CanFrameStore {
  canfd_frame frame;
  uint64_t timestamp{0};
  bool hasBeenRead{false};
};

class CanStream {
  struct private_init {};

 public:
  static std::shared_ptr<CanStream> Construct(uint8_t busId, HAL_CANManufacturer manufacturer,
                                              HAL_CANDeviceType deviceType,
                                              uint8_t deviceId);

  explicit CanStream(const private_init&) {};

  void InsertNewFrame(uint16_t api, const canfd_frame& frame, uint64_t timestamp);

  std::optional<CanFrameStore> ReadFrame(uint16_t api);

 private:
  uint8_t busId;
  wpi::mutex dataMutex;
  wpi::Event newDataEvent;
  // API Id
  wpi::SmallDenseMap<uint16_t, CanFrameStore> frames;
};

};  // namespace hal

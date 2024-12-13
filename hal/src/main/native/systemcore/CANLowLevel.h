#pragma once

#include <wpi/Synchronization.h>

#include <memory>
#include <hal/CANAPI.h>
#include <wpi/mutex.h>
#include <wpi/DenseMap.h>
#include <optional>
#include <vector>

#include <linux/can.h>

namespace hal {

struct CanFrame {
  bool isFd{false};
  bool isRtr{false};
  uint8_t length;
  uint8_t data[CANFD_MAX_DLEN] __attribute__((aligned(8)));
};

struct ReceivedCanFrame {
  uint64_t timestamp{0};
  bool hasBeenRead{false};
  CanFrame frame;
};

class MappedCanStream;
class AllCanStream;

class CanStream {
  struct private_init {};
  friend class MappedCanStream;
  friend class AllCanStream;

 public:
  static std::shared_ptr<MappedCanStream> ConstructMapped(
      uint8_t busId, HAL_CANManufacturer manufacturer,
      HAL_CANDeviceType deviceType, uint8_t deviceId);
  static std::shared_ptr<AllCanStream> ConstructAll(
      uint8_t busId, HAL_CANManufacturer manufacturer,
      HAL_CANDeviceType deviceType, uint8_t deviceId);

  virtual ~CanStream();

  virtual void InsertNewFrame(uint16_t apiId, const canfd_frame& frame,
                              uint64_t timestamp) = 0;

  int32_t WriteFrame(const CanFrame& frame, uint16_t apiId, int32_t periodMs);

  static int32_t WriteDirectFrame(const CanFrame& frame, uint8_t busId,
                                  uint32_t arbId, int32_t periodMs);

 protected:
  CanStream(const private_init&, uint8_t busId,
            HAL_CANManufacturer manufacturer, HAL_CANDeviceType deviceType,
            uint8_t deviceId)
      : busId{busId},
        manufacturer{manufacturer},
        deviceType{deviceType},
        deviceId{deviceId} {};

  const uint8_t busId;
  const HAL_CANManufacturer manufacturer;
  const HAL_CANDeviceType deviceType;
  const uint8_t deviceId;
};

class MappedCanStream : public CanStream {
 public:
  MappedCanStream(const private_init& token, uint8_t busId,
                  HAL_CANManufacturer manufacturer,
                  HAL_CANDeviceType deviceType, uint8_t deviceId)
      : CanStream{token, busId, manufacturer, deviceType, deviceId} {}

  std::optional<ReceivedCanFrame> ReadFrame(uint16_t apiId);

  void InsertNewFrame(uint16_t apiId, const canfd_frame& frame,
                      uint64_t timestamp) override;

 private:
  wpi::mutex dataMutex;
  // API Id is key
  wpi::SmallDenseMap<uint16_t, ReceivedCanFrame> frames;
};

class AllCanStream : public CanStream {
 public:
  AllCanStream(const private_init& token, uint8_t busId,
               HAL_CANManufacturer manufacturer, HAL_CANDeviceType deviceType,
               uint8_t deviceId)
      : CanStream{token, busId, manufacturer, deviceType, deviceId} {}

  void InsertNewFrame(uint16_t apiId, const canfd_frame& frame,
                      uint64_t timestamp) override;

  std::vector<ReceivedCanFrame> GetFrames();

 private:
  wpi::mutex dataMutex;
  wpi::Event newDataEvent{true};
  std::vector<ReceivedCanFrame> frames;
};

};  // namespace hal

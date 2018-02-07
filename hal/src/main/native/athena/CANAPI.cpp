/*----------------------------------------------------------------------------*/
/* Copyright (c) 2018 FIRST. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "HAL/CANAPI.h"

#include <llvm/MapVector.h>

#include "HAL/CAN.h"
#include "HAL/Errors.h"
#include "HAL/handles/UnlimitedHandleResource.h"

using namespace hal;

namespace {
struct Receives {
  uint64_t lastTimeStamp;
  uint8_t data[8];
  uint8_t length;
};

struct CANStorage {
  HAL_CANManufacturer manufacturer;
  HAL_CANDeviceType deviceType;
  uint8_t deviceId;
  wpi::mutex mapMutex;
  llvm::SmallDenseMap<int32_t, int32_t> periodicSends;
  llvm::SmallDenseMap<int32_t, Receives> receives;
};
}  // namespace

static UnlimitedHandleResource<HAL_CANHandle, CANStorage, HAL_HandleEnum::CAN>*
    canHandles;

namespace hal {
namespace init {
void InitializeCANAPI() {
  static UnlimitedHandleResource<HAL_CANHandle, CANStorage, HAL_HandleEnum::CAN>
      cH;
  canHandles = &cH;
}
}  // namespace init
}  // namespace hal

static int32_t CreateCANId(CANStorage* storage, int32_t id) { return 0; }

HAL_CANHandle HAL_InitializeCAN(HAL_CANManufacturer manufacturer,
                                int32_t deviceId, HAL_CANDeviceType deviceType,
                                int32_t* status) {
  auto can = std::make_shared<CANStorage>();

  auto handle = canHandles->Allocate(can);

  if (handle == HAL_kInvalidHandle) {
    *status = NO_AVAILABLE_RESOURCES;
    return HAL_kInvalidHandle;
  }

  can->deviceId = deviceId;
  can->deviceType = deviceType;
  can->manufacturer = manufacturer;

  return handle;
}

void HAL_CleanCAN(HAL_CANHandle handle) {
  auto data = canHandles->Free(handle);

  std::unique_lock<wpi::mutex> lock(data->mapMutex);

  for (auto&& i : data->periodicSends) {
    int32_t s = 0;
    HAL_CAN_SendMessage(i.first, nullptr, 0, HAL_CAN_SEND_PERIOD_STOP_REPEATING,
                        &s);
    i.second = -1;
  }
}

void HAL_WriteCANPacket(HAL_CANHandle handle, const uint8_t* data,
                        int32_t length, int32_t apiId, int32_t* status) {
  auto can = canHandles->Get(handle);
  if (!can) {
    *status = HAL_HANDLE_ERROR;
    return;
  }
  auto id = CreateCANId(can.get(), apiId);

  HAL_CAN_SendMessage(id, data, length, HAL_CAN_SEND_PERIOD_NO_REPEAT, status);

  if (*status != 0) {
    return;
  }
  std::unique_lock<wpi::mutex> lock(can->mapMutex);
  can->periodicSends[apiId] = -1;
}

void HAL_WriteCANPacketRepeating(HAL_CANHandle handle, const uint8_t* data,
                                 int32_t length, int32_t apiId,
                                 int32_t repeatMs, int32_t* status) {
  auto can = canHandles->Get(handle);
  if (!can) {
    *status = HAL_HANDLE_ERROR;
    return;
  }
  auto id = CreateCANId(can.get(), apiId);

  HAL_CAN_SendMessage(id, data, length, repeatMs, status);

  if (*status != 0) {
    return;
  }
  std::unique_lock<wpi::mutex> lock(can->mapMutex);
  can->periodicSends[apiId] = repeatMs;
}

void HAL_StopCANPacketRepeating(HAL_CANHandle handle, int32_t apiId,
                                int32_t* status) {
  auto can = canHandles->Get(handle);
  if (!can) {
    *status = HAL_HANDLE_ERROR;
    return;
  }
  auto id = CreateCANId(can.get(), apiId);

  HAL_CAN_SendMessage(id, nullptr, 0, HAL_CAN_SEND_PERIOD_STOP_REPEATING,
                      status);

  if (*status != 0) {
    return;
  }
  std::unique_lock<wpi::mutex> lock(can->mapMutex);
  can->periodicSends[apiId] = -1;
}

void HAL_ReadCANPacketNew(HAL_CANHandle handle, int32_t apiId, uint8_t* data,
                          int32_t* length, uint64_t* receivedTimestamp,
                          int32_t* status) {
  auto can = canHandles->Get(handle);
  if (!can) {
    *status = HAL_HANDLE_ERROR;
    return;
  }
  auto id = CreateCANId(can.get(), apiId);

  uint32_t messageId = 0;
  uint8_t dataSize = 0;
  uint32_t timestamp = 0;
  HAL_CAN_ReceiveMessage(&messageId, id, data, &dataSize, &timestamp, status);

  *length = dataSize;
  *receivedTimestamp = timestamp;  // TODO : Fix this
}

void HAL_ReadCANPacketLatest(HAL_CANHandle handle, int32_t apiId, uint8_t* data,
                             int32_t* length, uint64_t* receivedTimestamp,
                             int32_t* status) {
  auto can = canHandles->Get(handle);
  if (!can) {
    *status = HAL_HANDLE_ERROR;
    return;
  }
  auto id = CreateCANId(can.get(), apiId);

  uint32_t messageId = 0;
  uint8_t dataSize = 0;
  uint32_t timestamp = 0;
  HAL_CAN_ReceiveMessage(&messageId, id, data, &dataSize, &timestamp, status);
  std::unique_lock<wpi::mutex> lock(can->mapMutex);

  if (status == 0) {
    // fresh update
    auto& msg = can->receives[id];
    msg.length = dataSize;
    *length = dataSize;
    msg.lastTimeStamp = timestamp;
    *receivedTimestamp = timestamp;
    std::memcpy(msg.data, data, dataSize);
  } else {
    auto i = can->receives.find(id);
    if (i != can->receives.end()) {
      std::memcpy(i->second.data, data, i->second.length);
      *length = i->second.length;
      *receivedTimestamp = i->second.lastTimeStamp;
      *status = 0;
    }
  }
}

void HAL_ReadCANPacketTimeout(HAL_CANHandle handle, int32_t apiId,
                              uint8_t* data, int32_t* length,
                              uint64_t* receivedTimestamp, int32_t timeoutMs,
                              int32_t* status) {
  auto can = canHandles->Get(handle);
  if (!can) {
    *status = HAL_HANDLE_ERROR;
    return;
  }
  auto id = CreateCANId(can.get(), apiId);

  uint32_t messageId = 0;
  uint8_t dataSize = 0;
  uint32_t timestamp = 0;
  HAL_CAN_ReceiveMessage(&messageId, id, data, &dataSize, &timestamp, status);
  std::unique_lock<wpi::mutex> lock(can->mapMutex);

  if (status == 0) {
    // fresh update
    auto& msg = can->receives[id];
    msg.length = dataSize;
    *length = dataSize;
    msg.lastTimeStamp = timestamp;
    *receivedTimestamp = timestamp;
    std::memcpy(msg.data, data, dataSize);
  } else {
    auto i = can->receives.find(id);
    if (i != can->receives.end()) {
      // Found, check if new enough
      uint64_t now = 0;  // TODO: fix
      if (now - i->second.lastTimeStamp >
          static_cast<uint64_t>(timeoutMs) * 1000) {
        // Timeout, return bad status
        return;
      }
      std::memcpy(i->second.data, data, i->second.length);
      *length = i->second.length;
      *receivedTimestamp = i->second.lastTimeStamp;
      *status = 0;
    }
  }
}

// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

#include "hal/CANAPI.h"

#include "CANLowLevel.h"

#include <ctime>
#include <memory>

#include <wpi/DenseMap.h>
#include <wpi/mutex.h>
#include <wpi/timestamp.h>

#include "HALInitializer.h"
#include "hal/CAN.h"
#include "hal/Errors.h"
#include "hal/handles/UnlimitedHandleResource.h"

using namespace hal;

namespace {
struct Receives {
  uint32_t lastTimeStamp;
  uint8_t data[8];
  uint8_t length;
};

struct CANStorage {
  std::shared_ptr<MappedCanStream> stream;
  HAL_CANManufacturer manufacturer;
  HAL_CANDeviceType deviceType;
  uint8_t deviceId;
  wpi::mutex periodicSendsMutex;
  wpi::SmallDenseMap<int32_t, int32_t> periodicSends;
};
}  // namespace

static UnlimitedHandleResource<HAL_CANHandle, CANStorage, HAL_HandleEnum::CAN>*
    canHandles;

namespace hal::init {
void InitializeCANAPI() {
  static UnlimitedHandleResource<HAL_CANHandle, CANStorage, HAL_HandleEnum::CAN>
      cH;
  canHandles = &cH;
}
}  // namespace hal::init

static int32_t CreateCANId(CANStorage* storage, int32_t apiId) {
  int32_t createdId = 0;
  createdId |= (static_cast<int32_t>(storage->deviceType) & 0x1F) << 24;
  createdId |= (static_cast<int32_t>(storage->manufacturer) & 0xFF) << 16;
  createdId |= (apiId & 0x3FF) << 6;
  createdId |= (storage->deviceId & 0x3F);
  return createdId;
}

extern "C" {

uint32_t HAL_GetCANPacketBaseTime(void) {
  return wpi::Now();
}

HAL_CANHandle HAL_InitializeCAN(HAL_CANManufacturer manufacturer,
                                int32_t deviceId, HAL_CANDeviceType deviceType,
                                int32_t* status) {
  hal::init::CheckInit();
  auto can = std::make_shared<CANStorage>();

  auto handle = canHandles->Allocate(can);

  if (handle == HAL_kInvalidHandle) {
    *status = NO_AVAILABLE_RESOURCES;
    return HAL_kInvalidHandle;
  }

  can->stream =
      CanStream::ConstructMapped(0, manufacturer, deviceType, deviceId);
  if (!can->stream) {
    *status = PARAMETER_OUT_OF_RANGE;
    canHandles->Free(handle);
    return HAL_kInvalidHandle;
  }

  can->deviceId = deviceId;
  can->deviceType = deviceType;
  can->manufacturer = manufacturer;

  return handle;
}

void HAL_CleanCAN(HAL_CANHandle handle) {
  auto data = canHandles->Free(handle);
  if (data == nullptr) {
    return;
  }

  std::scoped_lock lock(data->periodicSendsMutex);

  hal::CanFrame frame;

  for (auto&& i : data->periodicSends) {
    data->stream->WriteFrame(frame, i.first,
                             HAL_CAN_SEND_PERIOD_STOP_REPEATING);
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

  hal::CanFrame frame;
  frame.length = (std::min)(length, CAN_MAX_DLEN);
  memcpy(frame.data, data, frame.length);

  std::scoped_lock lock(can->periodicSendsMutex);
  *status =
      can->stream->WriteFrame(frame, apiId, HAL_CAN_SEND_PERIOD_NO_REPEAT);
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

  hal::CanFrame frame;
  frame.length = (std::min)(length, CAN_MAX_DLEN);
  memcpy(frame.data, data, frame.length);

  std::scoped_lock lock(can->periodicSendsMutex);
  *status = can->stream->WriteFrame(frame, apiId, repeatMs);
  can->periodicSends[apiId] = repeatMs;
}

void HAL_WriteCANRTRFrame(HAL_CANHandle handle, int32_t length, int32_t apiId,
                          int32_t* status) {
  auto can = canHandles->Get(handle);
  if (!can) {
    *status = HAL_HANDLE_ERROR;
    return;
  }

  hal::CanFrame frame;
  frame.isRtr = true;
  frame.length = (std::min)(length, CAN_MAX_DLEN);

  std::scoped_lock lock(can->periodicSendsMutex);
  *status =
      can->stream->WriteFrame(frame, apiId, HAL_CAN_SEND_PERIOD_NO_REPEAT);
  can->periodicSends[apiId] = -1;
}

void HAL_StopCANPacketRepeating(HAL_CANHandle handle, int32_t apiId,
                                int32_t* status) {
  auto can = canHandles->Get(handle);
  if (!can) {
    *status = HAL_HANDLE_ERROR;
    return;
  }

  hal::CanFrame frame;

  std::scoped_lock lock(can->periodicSendsMutex);
  *status =
      can->stream->WriteFrame(frame, apiId, HAL_CAN_SEND_PERIOD_STOP_REPEATING);
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

  std::optional<ReceivedCanFrame> maybeMessage = can->stream->ReadFrame(apiId);
  if (!maybeMessage.has_value() || maybeMessage->timestamp == 0 ||
      maybeMessage->hasBeenRead) {
    *status = HAL_ERR_CANSessionMux_MessageNotFound;
    return;
  }

  if (maybeMessage->frame.isFd) {
    printf("FD frames not supported yet\n");
    *status = INCOMPATIBLE_STATE;
    return;
  }

  std::memcpy(data, maybeMessage->frame.data, maybeMessage->frame.length);
  *length = maybeMessage->frame.length;
  *receivedTimestamp = maybeMessage->timestamp;
  *status = 0;
}

void HAL_ReadCANPacketLatest(HAL_CANHandle handle, int32_t apiId, uint8_t* data,
                             int32_t* length, uint64_t* receivedTimestamp,
                             int32_t* status) {
  auto can = canHandles->Get(handle);
  if (!can) {
    *status = HAL_HANDLE_ERROR;
    return;
  }

  std::optional<ReceivedCanFrame> maybeMessage = can->stream->ReadFrame(apiId);
  if (!maybeMessage.has_value() || maybeMessage->timestamp == 0) {
    *status = HAL_ERR_CANSessionMux_MessageNotFound;
    return;
  }

  if (maybeMessage->frame.isFd) {
    printf("FD frames not supported yet\n");
    *status = INCOMPATIBLE_STATE;
    return;
  }

  std::memcpy(data, maybeMessage->frame.data, maybeMessage->frame.length);
  *length = maybeMessage->frame.length;
  *receivedTimestamp = maybeMessage->timestamp;
  *status = 0;
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

  std::optional<ReceivedCanFrame> maybeMessage = can->stream->ReadFrame(apiId);
  if (!maybeMessage.has_value() || maybeMessage->timestamp == 0) {
    *status = HAL_ERR_CANSessionMux_MessageNotFound;
    return;
  }

  if (maybeMessage->frame.isFd) {
    printf("FD frames not supported yet\n");
    *status = INCOMPATIBLE_STATE;
    return;
  }

  auto now = wpi::Now();

  if (now - maybeMessage->timestamp > static_cast<uint64_t>(timeoutMs) * 1000) {
    *status = HAL_CAN_TIMEOUT;
    return;
  }

  std::memcpy(data, maybeMessage->frame.data, maybeMessage->frame.length);
  *length = maybeMessage->frame.length;
  *receivedTimestamp = maybeMessage->timestamp;
  *status = 0;
  return;
}

uint32_t HAL_StartCANStream(HAL_CANHandle handle, int32_t apiId, int32_t depth,
                            int32_t* status) {
  auto can = canHandles->Get(handle);
  if (!can) {
    *status = HAL_HANDLE_ERROR;
    return 0;
  }

  uint32_t messageId = CreateCANId(can.get(), apiId);

  uint32_t session = 0;
  HAL_CAN_OpenStreamSession(&session, messageId, 0x1FFFFFFF, depth, status);
  return session;
}
}  // extern "C"

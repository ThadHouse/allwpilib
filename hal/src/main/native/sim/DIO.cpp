/*----------------------------------------------------------------------------*/
/* Copyright (c) 2016-2018 FIRST. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "HAL/DIO.h"

#include <cmath>

#include "DigitalInternal.h"
#include "HAL/handles/HandlesInternal.h"
#include "HAL/handles/LimitedHandleResource.h"
#include "HALInitializer.h"
#include "MockData/DIODataInternal.h"
#include "MockData/DigitalPWMDataInternal.h"
#include "PortsInternal.h"

using namespace hal;

static LimitedHandleResource<HAL_DigitalPWMHandle, uint8_t,
                             kNumDigitalPWMOutputs, HAL_HandleEnum::DigitalPWM>*
    digitalPWMHandles;

namespace hal {
namespace init {
void InitializeDIO() {
  static LimitedHandleResource<HAL_DigitalPWMHandle, uint8_t,
                               kNumDigitalPWMOutputs,
                               HAL_HandleEnum::DigitalPWM>
      dpH;
  digitalPWMHandles = &dpH;
}
}  // namespace init
}  // namespace hal

extern "C" {

HAL_DigitalHandle HAL_InitializeDIOPort(HAL_PortHandle portHandle,
                                        HAL_Bool input, int32_t* status) {
  hal::init::CheckInit();
  if (*status != 0) return HAL_kInvalidHandle;

  int16_t channel = getPortHandleChannel(portHandle);
  if (channel == InvalidHandleIndex) {
    *status = PARAMETER_OUT_OF_RANGE;
    return HAL_kInvalidHandle;
  }

  auto handle =
      digitalChannelHandles->Allocate(channel, HAL_HandleEnum::DIO, status);

  if (*status != 0)
    return HAL_kInvalidHandle;  // failed to allocate. Pass error back.

  auto port = digitalChannelHandles->Get(handle, HAL_HandleEnum::DIO);
  if (port == nullptr) {  // would only occur on thread issue.
    *status = HAL_HANDLE_ERROR;
    return HAL_kInvalidHandle;
  }

  port->channel = static_cast<uint8_t>(channel);

  SimDIOData[channel].SetInitialized(true);

  SimDIOData[channel].SetIsInput(input);

  return handle;
}

HAL_Bool HAL_CheckDIOChannel(int32_t channel) {
  return channel < kNumDigitalChannels && channel >= 0;
}

void HAL_FreeDIOPort(HAL_DigitalHandle dioPortHandle) {
  auto port = digitalChannelHandles->Get(dioPortHandle, HAL_HandleEnum::DIO);
  // no status, so no need to check for a proper free.
  digitalChannelHandles->Free(dioPortHandle, HAL_HandleEnum::DIO);
  if (port == nullptr) return;
  SimDIOData[port->channel].SetInitialized(true);
}

HAL_DigitalPWMHandle HAL_AllocateDigitalPWM(int32_t* status) {
  auto handle = digitalPWMHandles->Allocate();
  if (handle == HAL_kInvalidHandle) {
    *status = NO_AVAILABLE_RESOURCES;
    return HAL_kInvalidHandle;
  }

  auto id = digitalPWMHandles->Get(handle);
  if (id == nullptr) {  // would only occur on thread issue.
    *status = HAL_HANDLE_ERROR;
    return HAL_kInvalidHandle;
  }
  *id = static_cast<uint8_t>(getHandleIndex(handle));

  SimDigitalPWMData[*id].SetInitialized(true);

  return handle;
}

void HAL_FreeDigitalPWM(HAL_DigitalPWMHandle pwmGenerator, int32_t* status) {
  auto port = digitalPWMHandles->Get(pwmGenerator);
  digitalPWMHandles->Free(pwmGenerator);
  if (port == nullptr) return;
  int32_t id = *port;
  SimDigitalPWMData[id].SetInitialized(false);
}

void HAL_SetDigitalPWMRate(double rate, int32_t* status) {
  // Currently rounding in the log rate domain... heavy weight toward picking a
  // higher freq.
  // TODO: Round in the linear rate domain.
  // uint8_t pwmPeriodPower = static_cast<uint8_t>(
  //    std::log(1.0 / (kExpectedLoopTiming * 0.25E-6 * rate)) /
  //        std::log(2.0) +
  //    0.5);
  // TODO(THAD) : Add a case to set this in the simulator
  // digitalSystem->writePWMPeriodPower(pwmPeriodPower, status);
}

void HAL_SetDigitalPWMDutyCycle(HAL_DigitalPWMHandle pwmGenerator,
                                double dutyCycle, int32_t* status) {
  auto port = digitalPWMHandles->Get(pwmGenerator);
  if (port == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return;
  }
  int32_t id = *port;
  if (dutyCycle > 1.0) dutyCycle = 1.0;
  if (dutyCycle < 0.0) dutyCycle = 0.0;
  SimDigitalPWMData[id].SetDutyCycle(dutyCycle);
}

void HAL_SetDigitalPWMOutputChannel(HAL_DigitalPWMHandle pwmGenerator,
                                    int32_t channel, int32_t* status) {
  auto port = digitalPWMHandles->Get(pwmGenerator);
  if (port == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return;
  }
  int32_t id = *port;
  SimDigitalPWMData[id].SetPin(channel);
}

void HAL_SetDIO(HAL_DigitalHandle dioPortHandle, HAL_Bool value,
                int32_t* status) {
  auto port = digitalChannelHandles->Get(dioPortHandle, HAL_HandleEnum::DIO);
  if (port == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return;
  }
  if (value != 0 && value != 1) {
    if (value != 0) value = 1;
  }
  SimDIOData[port->channel].SetValue(value);
}

void HAL_SetDIODirection(HAL_DigitalHandle dioPortHandle, HAL_Bool input,
                         int32_t* status) {
  auto port = digitalChannelHandles->Get(dioPortHandle, HAL_HandleEnum::DIO);
  if (port == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return;
  }

  SimDIOData[port->channel].SetIsInput(input);
}

HAL_Bool HAL_GetDIO(HAL_DigitalHandle dioPortHandle, int32_t* status) {
  auto port = digitalChannelHandles->Get(dioPortHandle, HAL_HandleEnum::DIO);
  if (port == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return false;
  }
  HAL_Bool value = SimDIOData[port->channel].GetValue();
  if (value > 1) value = 1;
  if (value < 0) value = 0;
  return value;
}

HAL_Bool HAL_GetDIODirection(HAL_DigitalHandle dioPortHandle, int32_t* status) {
  auto port = digitalChannelHandles->Get(dioPortHandle, HAL_HandleEnum::DIO);
  if (port == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return false;
  }
  HAL_Bool value = SimDIOData[port->channel].GetIsInput();
  if (value > 1) value = 1;
  if (value < 0) value = 0;
  return value;
}

void HAL_Pulse(HAL_DigitalHandle dioPortHandle, double pulseLength,
               int32_t* status) {
  auto port = digitalChannelHandles->Get(dioPortHandle, HAL_HandleEnum::DIO);
  if (port == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return;
  }
  // TODO (Thad) Add this
}

HAL_Bool HAL_IsPulsing(HAL_DigitalHandle dioPortHandle, int32_t* status) {
  auto port = digitalChannelHandles->Get(dioPortHandle, HAL_HandleEnum::DIO);
  if (port == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return false;
  }
  return false;
  // TODO (Thad) Add this
}

HAL_Bool HAL_IsAnyPulsing(int32_t* status) {
  return false;  // TODO(Thad) Figure this out
}

HAL_FilterHandle HAL_CreateFilterForDIO(HAL_DigitalHandle dioPortHandle,
                                        int32_t* status) {
  // TODO(Thad) figure this out
  return HAL_kInvalidHandle;
}

void HAL_CleanFilter(HAL_FilterHandle handle) {
  // TODO(Thad) figure this out
}

void HAL_SetFilterPeriod(HAL_FilterHandle handle, int64_t value,
                         int32_t* status) {
  // TODO(Thad) figure this out
}

int64_t HAL_GetFilterPeriod(HAL_FilterHandle handle, int32_t* status) {
  return 0;  // TODO(Thad) figure this out
}
}  // extern "C"

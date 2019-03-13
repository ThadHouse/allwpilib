/*----------------------------------------------------------------------------*/
/* Copyright (c) 2016-2018 FIRST. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "hal/Solenoid.h"

#include <FRC_NetworkCommunication/LoadOut.h>

#include "HALInitializer.h"
#include "PortsInternal.h"
#include "hal/ChipObject.h"
#include "hal/Errors.h"
#include "hal/Ports.h"
#include "hal/PCM.h"
#include "hal/handles/HandlesInternal.h"
#include "hal/handles/IndexedHandleResource.h"

namespace {

struct Solenoid {
  HAL_PCMHandle pcmHandle;
  uint8_t channel;
};

}  // namespace

using namespace hal;

static IndexedHandleResource<HAL_SolenoidHandle, Solenoid,
                             kNumSolenoidChannels * kNumPCMModules,
                             HAL_HandleEnum::Solenoid>* solenoidHandles;

namespace hal {
namespace init {
void InitializeSolenoid() {
  static IndexedHandleResource<HAL_SolenoidHandle, Solenoid,
                               kNumSolenoidChannels * kNumPCMModules,
                               HAL_HandleEnum::Solenoid>
      sH;
  solenoidHandles = &sH;
}
}  // namespace init
}  // namespace hal

extern "C" {

HAL_SolenoidHandle HAL_InitializeSolenoidPort(HAL_PortHandle portHandle,
                                              int32_t* status) {
  hal::init::CheckInit();
  int16_t channel = getPortHandleChannel(portHandle);
  int16_t module = getPortHandleModule(portHandle);
  if (channel == InvalidHandleIndex) {
    *status = HAL_HANDLE_ERROR;
    return HAL_kInvalidHandle;
  }

  // initializePCM will check the module
  if (!HAL_CheckSolenoidChannel(channel)) {
    *status = RESOURCE_OUT_OF_RANGE;
    return HAL_kInvalidHandle;
  }

  auto pcm = HAL_InitializePCM(module, status);
  if (*status != 0) {
    return HAL_kInvalidHandle;
  }

  auto handle = solenoidHandles->Allocate(module * kNumSolenoidChannels + channel, status);

  if(*status != 0) {
    HAL_CleanPCM(pcm);
    return HAL_kInvalidHandle;
  }

  auto data = solenoidHandles->Get(handle);
  if (data == nullptr) {
    *status = HAL_HANDLE_ERROR;
    HAL_CleanPCM(pcm);
    return HAL_kInvalidHandle;
  }

  data->channel = channel;
  data->pcmHandle = pcm;
}

HAL_SolenoidHandle HAL_InitializeSolenoidPortFromPCM(HAL_PCMHandle pcm, int32_t port,
                                              int32_t* status) {
  if (!HAL_CheckSolenoidChannel(port)) {
    *status = RESOURCE_OUT_OF_RANGE;
    return HAL_kInvalidHandle;
  }

  auto module = HAL_IncremementPCMRefCountAndGetModuleNumber(pcm, status);
  if (*status != 0) {
    return HAL_kInvalidHandle;
  }

  auto handle = solenoidHandles->Allocate(module * kNumSolenoidChannels + port, status);

  if(*status != 0) {
    HAL_CleanPCM(pcm);
    return HAL_kInvalidHandle;
  }

  auto data = solenoidHandles->Get(handle);
  if (data == nullptr) {
    *status = HAL_HANDLE_ERROR;
    HAL_CleanPCM(pcm);
    return HAL_kInvalidHandle;
  }

  data->channel = port;
  data->pcmHandle = pcm;
}

void HAL_FreeSolenoidPort(HAL_SolenoidHandle solenoidPortHandle) {
  auto data = solenoidHandles->Get(solenoidPortHandle);
  if (data == nullptr) return;
  solenoidHandles->Free(solenoidPortHandle);
  HAL_CleanPCM(data->pcmHandle);
}

HAL_Bool HAL_CheckSolenoidModule(int32_t module) {
  return module < kNumPCMModules && module >= 0;
}

HAL_Bool HAL_CheckSolenoidChannel(int32_t channel) {
  return channel < kNumSolenoidChannels && channel >= 0;
}

HAL_Bool HAL_GetSolenoid(HAL_SolenoidHandle solenoidPortHandle,
                         int32_t* status) {
  auto port = solenoidHandles->Get(solenoidPortHandle);
  if (port == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return false;
  }

  return HAL_GetPCMSolenoid(port->pcmHandle, port->channel, status);
}

void HAL_SetSolenoid(HAL_SolenoidHandle solenoidPortHandle, HAL_Bool value,
                     int32_t* status) {
  auto port = solenoidHandles->Get(solenoidPortHandle);
  if (port == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return;
  }

  HAL_SetPCMSolenoid(port->pcmHandle, port->channel, value, status);
}

void HAL_SetOneShotDuration(HAL_SolenoidHandle solenoidPortHandle,
                            int32_t durMS, int32_t* status) {
  auto port = solenoidHandles->Get(solenoidPortHandle);
  if (port == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return;
  }

  HAL_SetPCMOneShotDuration(port->pcmHandle, port->channel, durMS, status);
}

void HAL_FireOneShot(HAL_SolenoidHandle solenoidPortHandle, int32_t* status) {
  auto port = solenoidHandles->Get(solenoidPortHandle);
  if (port == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return;
  }

  HAL_FirePCMOneShotSolenoid(port->pcmHandle, port->channel, status);
}
}  // extern "C"

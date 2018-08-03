/*----------------------------------------------------------------------------*/
/* Copyright (c) 2016-2018 FIRST. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "hal/PCM.h"

#include <memory>
#include <thread>

#include <wpi/raw_ostream.h>

#include "mockdata/PCMDataInternal.h"
#include "CANAPIInternal.h"
#include "HALInitializer.h"
#include "PortsInternal.h"
#include "hal/CANAPI.h"
#include "hal/Errors.h"
#include "hal/Ports.h"
#include "hal/cpp/fpga_clock.h"
#include "hal/handles/IndexedHandleResource.h"

using namespace hal;

static constexpr HAL_CANManufacturer manufacturer =
    HAL_CANManufacturer::HAL_CAN_Man_kCTRE;

static constexpr HAL_CANDeviceType deviceType =
    HAL_CANDeviceType::HAL_CAN_Dev_kPneumatics;

namespace {
struct PCMDataStore {
  HAL_PCMHandle handle;
  std::atomic_int refCount{0};
};
}  // namespace

static IndexedHandleResource<HAL_PCMHandle, PCMDataStore, kNumPCMModules,
                             HAL_HandleEnum::PCM>* pcmHandles;

namespace hal {
namespace init {
void InitializePCM() {
  static IndexedHandleResource<HAL_PCMHandle, PCMDataStore, kNumPCMModules,
                               HAL_HandleEnum::PCM>
      pH;
  pcmHandles = &pH;
}
}  // namespace init
}  // namespace hal


extern "C" {
HAL_PCMHandle HAL_GetPCMHandle(int32_t module, int32_t* status) {
  hal::init::CheckInit();
  if (!HAL_CheckPCMModule(module)) {
    *status = PARAMETER_OUT_OF_RANGE;
    return HAL_kInvalidHandle;
  }
  auto handlePtr = pcmHandles->GetOrAllocate(module, status);
  if (*status != 0) {
    return HAL_kInvalidHandle;
  }

  if (handlePtr.second->refCount.fetch_add(1) == 0) {
    // Newly allocated, initialize
    handlePtr.second->handle =
        HAL_InitializeCAN(manufacturer, module, deviceType, status);

    SimPCMData[module].SetInitialized(true);

    if (*status != 0) {
      pcmHandles->Free(handlePtr.first);
      return HAL_kInvalidHandle;
    }
  }

  return handlePtr.first;
}

void HAL_FreePCMHandle(HAL_PCMHandle handle) {
  auto ptr = pcmHandles->Get(handle);
  if (!ptr) {
    return;
  }

  if (ptr->refCount.fetch_sub(1) == 1) {
    pcmHandles->Free(handle);

    // Wait for no other object to hold this handle.
    auto start = hal::fpga_clock::now();
    while (ptr.use_count() != 1) {
      auto current = hal::fpga_clock::now();
      if (start + std::chrono::seconds(1) < current) {
        wpi::outs() << "DIO handle free timeout\n";
        wpi::outs().flush();
        break;
      }
      std::this_thread::yield();
    }
    HAL_CleanCAN(ptr->handle);
    ptr->handle = HAL_kInvalidHandle;
    int32_t status = 0;
      auto module = hal::can::GetCANModuleFromHandle(ptr->handle, &status);
  if (status == 0) {
    SimPCMData[module].SetInitialized(false);
  }

  }
}

HAL_Bool HAL_CheckPCMChannel(int32_t channel) {
  return channel < kNumSolenoidChannels && channel >= 0;
}

HAL_Bool HAL_CheckPCMModule(int32_t module) {
  return module < kNumPCMModules && module >= 0;
}

double HAL_GetPCMInputVoltage(HAL_PCMHandle handle, int32_t* status) {
  return 0.0;
}

double HAL_GetPCMSolenoidVoltage(HAL_PCMHandle handle, int32_t* status) {
  return 0.0;
}

HAL_Bool HAL_GetPCMSolenoid(HAL_PCMHandle handle, int32_t index, int32_t* status) {
  auto pcm = pcmHandles->Get(handle);
  if (!pcm) {
    *status = HAL_HANDLE_ERROR;
    return false;
  }
  auto module = hal::can::GetCANModuleFromHandle(pcm->handle, status);
  if (*status != 0) {
    return false;
  }
  return SimPCMData[module].GetSolenoidOutput(index);
}

int32_t HAL_GetPCMAllSolenoids(HAL_PCMHandle handle, int32_t* status) {
  auto pcm = pcmHandles->Get(handle);
  if (!pcm) {
    *status = HAL_HANDLE_ERROR;
    return false;
  }
  auto module = hal::can::GetCANModuleFromHandle(pcm->handle, status);
  if (*status != 0) {
    return false;
  }
  return SimPCMData[module].GetAllSolenoidOutputs();
}

void HAL_SetPCMSolenoid(HAL_PCMHandle handle, int32_t index, HAL_Bool value,
                        int32_t* status) {
  auto pcm = pcmHandles->Get(handle);
  if (!pcm) {
    *status = HAL_HANDLE_ERROR;
    return;
  }
  auto module = hal::can::GetCANModuleFromHandle(pcm->handle, status);
  if (*status != 0) {
    return;
  }
  SimPCMData[module].SetSolenoidOutput(index, value);
}

void HAL_SetPCMAllSolenoids(HAL_PCMHandle handle, int32_t values,
                            int32_t* status) {
  auto pcm = pcmHandles->Get(handle);
  if (!pcm) {
    *status = HAL_HANDLE_ERROR;
    return;
  }
  auto module = hal::can::GetCANModuleFromHandle(pcm->handle, status);
  if (*status != 0) {
    return;
  }
  SimPCMData[module].SetAllSolenoidOutputs(values);
}

void HAL_SetPCMOneShotDuration(HAL_PCMHandle handle, int32_t index,
                               int32_t durMS, int32_t* status) {
}

void HAL_FirePCMOneShot(HAL_PCMHandle handle, int32_t index, int32_t* status) {
}

HAL_Bool HAL_GetPCMCompressorOn(HAL_PCMHandle handle, int32_t* status) {
    auto pcm = pcmHandles->Get(handle);
  if (!pcm) {
    *status = HAL_HANDLE_ERROR;
    return false;
  }
  auto module = hal::can::GetCANModuleFromHandle(pcm->handle, status);
  if (*status != 0) {
    return false;
  }
  return SimPCMData[module].GetCompressorOn();
}

void HAL_SetPCMCompressorClosedLoopControl(HAL_PCMHandle handle, HAL_Bool value,
                                           int32_t* status) {
    auto pcm = pcmHandles->Get(handle);
  if (!pcm) {
    *status = HAL_HANDLE_ERROR;
    return;
  }
  auto module = hal::can::GetCANModuleFromHandle(pcm->handle, status);
  if (*status != 0) {
    return;
  }
  SimPCMData[module].SetClosedLoopEnabled(value);
}

HAL_Bool HAL_GetPCMCompressorClosedLoopControl(HAL_PCMHandle handle,
                                               int32_t* status) {
    auto pcm = pcmHandles->Get(handle);
  if (!pcm) {
    *status = HAL_HANDLE_ERROR;
    return false;
  }
  auto module = hal::can::GetCANModuleFromHandle(pcm->handle, status);
  if (*status != 0) {
    return false;
  }
  return SimPCMData[module].GetClosedLoopEnabled();
}

HAL_Bool HAL_GetPCMCompressorPressureSwitch(HAL_PCMHandle handle,
                                            int32_t* status) {
    auto pcm = pcmHandles->Get(handle);
  if (!pcm) {
    *status = HAL_HANDLE_ERROR;
    return false;
  }
  auto module = hal::can::GetCANModuleFromHandle(pcm->handle, status);
  if (*status != 0) {
    return false;
  }
  return SimPCMData[module].GetPressureSwitch();
}

double HAL_GetPCMCompressorCurrent(HAL_PCMHandle handle, int32_t* status) {
    auto pcm = pcmHandles->Get(handle);
  if (!pcm) {
    *status = HAL_HANDLE_ERROR;
    return 0;
  }
  auto module = hal::can::GetCANModuleFromHandle(pcm->handle, status);
  if (*status != 0) {
    return 0;
  }
  return SimPCMData[module].GetCompressorCurrent();
}

HAL_Bool HAL_GetPCMCompressorCurrentTooHighFault(HAL_PCMHandle handle,
                                                 int32_t* status) {
                                                   return false;
}

HAL_Bool HAL_GetPCMCompressorCurrentTooHighStickyFault(HAL_PCMHandle handle,
                                                       int32_t* status) {
                                                         return false;
}

HAL_Bool HAL_GetPCMCompressorShortedStickyFault(HAL_PCMHandle handle,
                                                int32_t* status) {
                                                  return false;
}

HAL_Bool HAL_GetPCMCompressorShortedFault(HAL_PCMHandle handle,
                                          int32_t* status) {
                                            return false;
}

HAL_Bool HAL_GetPCMCompressorNotConnectedStickyFault(HAL_PCMHandle handle,
                                                     int32_t* status) {
                                                       return false;
}

HAL_Bool HAL_GetPCMCompressorNotConnectedFault(HAL_PCMHandle handle,
                                               int32_t* status) {
                                                 return false;
}

int32_t HAL_GetPCMSolenoidBlackList(HAL_PCMHandle handle, int32_t* status) {
  return 0;
}

HAL_Bool HAL_GetPCMSolenoidVoltageStickyFault(HAL_PCMHandle handle,
                                              int32_t* status) {
                                                return false;
}

HAL_Bool HAL_GetPCMSolenoidVoltageFault(HAL_PCMHandle handle, int32_t* status) {
  return false;
}

void HAL_ClearAllPCMStickyFaults(HAL_PCMHandle handle, int32_t* status) {
}
}  // extern "C"

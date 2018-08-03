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

static constexpr int32_t Status1 = 0x50;
static constexpr int32_t StatusSolFaults = 0x51;
static constexpr int32_t StatusDebug = 0x52;

static constexpr int32_t Control1 = 0x50;
static constexpr int32_t Control2 = 0x51;
static constexpr int32_t Control3 = 0x52;

static constexpr int32_t TimeoutMs = 50;
static constexpr int32_t StatusPeriodMs = 25;
static constexpr int32_t ControlSendPeriod = 20;

union PcmStatus1 {
  uint8_t data[8];
  struct Bits {
    /* Byte 0 */
    unsigned SolenoidBits : 8;
    /* Byte 1 */
    unsigned compressorOn : 1;
    unsigned stickyFaultFuseTripped : 1;
    unsigned stickyFaultCompCurrentTooHigh : 1;
    unsigned faultFuseTripped : 1;
    unsigned faultCompCurrentTooHigh : 1;
    unsigned faultHardwareFailure : 1;
    unsigned isCloseloopEnabled : 1;
    unsigned pressureSwitchEn : 1;
    /* Byte 2*/
    unsigned battVoltage : 8;
    /* Byte 3 */
    unsigned solenoidVoltageTop8 : 8;
    /* Byte 4 */
    unsigned compressorCurrentTop6 : 6;
    unsigned solenoidVoltageBtm2 : 2;
    /* Byte 5 */
    unsigned StickyFault_dItooHigh : 1;
    unsigned Fault_dItooHigh : 1;
    unsigned moduleEnabled : 1;
    unsigned closedLoopOutput : 1;
    unsigned compressorCurrentBtm4 : 4;
    /* Byte 6 */
    unsigned tokenSeedTop8 : 8;
    /* Byte 7 */
    unsigned tokenSeedBtm8 : 8;
  } bits;
};

union PcmStatusFaults {
  uint8_t data[8];
  struct Bits {
    /* Byte 0 */
    unsigned SolenoidBlacklist : 8;
    /* Byte 1 */
    unsigned reserved_bit0 : 1;
    unsigned reserved_bit1 : 1;
    unsigned reserved_bit2 : 1;
    unsigned reserved_bit3 : 1;
    unsigned StickyFault_CompNoCurrent : 1;
    unsigned Fault_CompNoCurrent : 1;
    unsigned StickyFault_SolenoidJumper : 1;
    unsigned Fault_SolenoidJumper : 1;
  } bits;
};

union PcmDebug {
  uint8_t data[8];
  struct Bits {
    unsigned tokFailsTop8 : 8;
    unsigned tokFailsBtm8 : 8;
    unsigned lastFailedTokTop8 : 8;
    unsigned lastFailedTokBtm8 : 8;
    unsigned tokSuccessTop8 : 8;
    unsigned tokSuccessBtm8 : 8;
  } bits;
};

union PcmControl1 {
  uint8_t data[8];
  struct Bits {
    /* Byte 0 */
    unsigned tokenTop8 : 8;
    /* Byte 1 */
    unsigned tokenBtm8 : 8;
    /* Byte 2 */
    unsigned solenoidBits : 8;
    /* Byte 3*/
    unsigned reserved : 4;
    unsigned closeLoopOutput : 1;
    unsigned compressorOn : 1;
    unsigned closedLoopEnable : 1;
    unsigned clearStickyFaults : 1;
    /* Byte 4 */
    unsigned OneShotField_h8 : 8;
    /* Byte 5 */
    unsigned OneShotField_l8 : 8;
  } bits;
};

struct PcmControlOneShot {
  uint8_t sol10MsPerUnit[8];
};

namespace {
struct PCMData {
  HAL_PCMHandle handle;
  std::atomic_int refCount{0};
  wpi::mutex dataMutex;
  PcmControlOneShot oneShotData;
  PcmControl1 controlData;
};
}  // namespace

static IndexedHandleResource<HAL_PCMHandle, PCMData, kNumPCMModules,
                             HAL_HandleEnum::PCM>* pcmHandles;

namespace hal {
namespace init {
void InitializePCM() {
  static IndexedHandleResource<HAL_PCMHandle, PCMData, kNumPCMModules,
                               HAL_HandleEnum::PCM>
      pH;
  pcmHandles = &pH;
}
}  // namespace init
}  // namespace hal

static void SendControl(PCMData* data, int32_t* status) {
  HAL_WriteCANPacketRepeating(data->handle, data->controlData.data, 8, Control1, ControlSendPeriod, status);
}

static void SendOneShot(PCMData* data, int32_t* status) {
HAL_WriteCANPacketRepeating(data->handle, data->oneShotData.sol10MsPerUnit, 8, Control3, ControlSendPeriod, status);
}

static PcmStatus1 GetStatus1(HAL_PCMHandle handle, int32_t* status) {
  auto pcm = pcmHandles->Get(handle);
  if (!pcm) {
    *status = HAL_HANDLE_ERROR;
    return {};
  }
  PcmStatus1 pcmStatus;
  int32_t length = 0;
  uint64_t receivedTimestamp = 0;

  HAL_ReadCANPeriodicPacket(pcm->handle, Status1, pcmStatus.data, &length,
                            &receivedTimestamp, TimeoutMs, StatusPeriodMs,
                            status);
  return pcmStatus;
}

static PcmStatusFaults GetStatusSolFault(HAL_PCMHandle handle,
                                         int32_t* status) {
  auto pcm = pcmHandles->Get(handle);
  if (!pcm) {
    *status = HAL_HANDLE_ERROR;
    return {};
  }
  PcmStatusFaults pcmStatus;
  int32_t length = 0;
  uint64_t receivedTimestamp = 0;

  HAL_ReadCANPeriodicPacket(pcm->handle, StatusSolFaults, pcmStatus.data,
                            &length, &receivedTimestamp, TimeoutMs,
                            StatusPeriodMs, status);
  return pcmStatus;
}

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
  }
}

HAL_Bool HAL_CheckPCMChannel(int32_t channel) {
  return channel < kNumSolenoidChannels && channel >= 0;
}

HAL_Bool HAL_CheckPCMModule(int32_t module) {
  return module < kNumPCMModules && module >= 0;
}

double HAL_GetPCMInputVoltage(HAL_PCMHandle handle, int32_t* status) {
  auto pcmStatus = GetStatus1(handle, status);
  if (*status != 0) return 0;

  return pcmStatus.bits.battVoltage * 0.05 + 4.0; /* 50mV per unit plus 4V. */
}

double HAL_GetPCMSolenoidVoltage(HAL_PCMHandle handle, int32_t* status) {
  auto pcmStatus = GetStatus1(handle, status);
  if (*status != 0) return 0;

  uint32_t raw = pcmStatus.bits.solenoidVoltageTop8;
  raw <<= 2;
  raw |= pcmStatus.bits.solenoidVoltageBtm2;
  return raw * 0.03125; /* 5.5 fixed pt value in Volts */
}

HAL_Bool HAL_GetPCMSolenoid(HAL_PCMHandle handle, int32_t index, int32_t* status) {
  auto pcmStatus = GetStatus1(handle, status);
  if (*status != 0) return 0;

  return (pcmStatus.bits.SolenoidBits & (1ul << (index))) ? 1 : 0;
}

int32_t HAL_GetPCMAllSolenoids(HAL_PCMHandle handle, int32_t* status) {
  auto pcmStatus = GetStatus1(handle, status);
  if (*status != 0) return 0;

  return pcmStatus.bits.SolenoidBits;
}

void HAL_SetPCMSolenoid(HAL_PCMHandle handle, int32_t index, HAL_Bool value,
                        int32_t* status) {
  auto pcm = pcmHandles->Get(handle);
  if (!pcm) {
    *status = HAL_HANDLE_ERROR;
    return;
  }
  std::unique_lock<wpi::mutex> lock(pcm->dataMutex);
  if (value) {
    pcm->controlData.bits.solenoidBits |= (1ul << (index));
  } else {
    pcm->controlData.bits.solenoidBits &= ~(1ul << (index));
  }
  SendControl(pcm.get(), status);
}

void HAL_SetPCMAllSolenoids(HAL_PCMHandle handle, int32_t values,
                            int32_t* status) {
  auto pcm = pcmHandles->Get(handle);
  if (!pcm) {
    *status = HAL_HANDLE_ERROR;
    return;
  }
  std::unique_lock<wpi::mutex> lock(pcm->dataMutex);
  pcm->controlData.bits.solenoidBits = values & 0xFF;
  SendControl(pcm.get(), status);
}

void HAL_SetPCMOneShotDuration(HAL_PCMHandle handle, int32_t index,
                               int32_t durMS, int32_t* status) {
  auto pcm = pcmHandles->Get(handle);
  if (!pcm) {
    *status = HAL_HANDLE_ERROR;
    return;
  }
  std::unique_lock<wpi::mutex> lock(pcm->dataMutex);
  pcm->oneShotData.sol10MsPerUnit[index] = std::min<int32_t>(durMS / 10, 0xFF);
  SendOneShot(pcm.get(), status);
}

void HAL_FirePCMOneShot(HAL_PCMHandle handle, int32_t index, int32_t* status) {
  auto pcm = pcmHandles->Get(handle);
  if (!pcm) {
    *status = HAL_HANDLE_ERROR;
    return;
  }
  std::unique_lock<wpi::mutex> lock(pcm->dataMutex);
  /* grab field as it is now */
  uint16_t oneShotField;
  oneShotField = pcm->controlData.bits.OneShotField_h8;
  oneShotField <<= 8;
  oneShotField |= pcm->controlData.bits.OneShotField_l8;
  /* get the caller's channel */
  uint16_t shift = 2 * index;
  uint16_t mask = 3; /* two bits wide */
  uint8_t chBits = (oneShotField >> shift) & mask;
  /* flip it */
  chBits = (chBits) % 3 + 1;
  /* clear out 2bits for this channel*/
  oneShotField &= ~(mask << shift);
  /* put new field in */
  oneShotField |= chBits << shift;
  /* apply field as it is now */
  pcm->controlData.bits.OneShotField_h8 = oneShotField >> 8;
  pcm->controlData.bits.OneShotField_l8 = oneShotField;
  SendControl(pcm.get(), status);
}

HAL_Bool HAL_GetPCMCompressorOn(HAL_PCMHandle handle, int32_t* status) {
  auto pcmStatus = GetStatus1(handle, status);
  if (*status != 0) return 0;

  return pcmStatus.bits.compressorOn != 0;
}

void HAL_SetPCMCompressorClosedLoopControl(HAL_PCMHandle handle, HAL_Bool value,
                                           int32_t* status) {
  auto pcm = pcmHandles->Get(handle);
  if (!pcm) {
    *status = HAL_HANDLE_ERROR;
    return;
  }
  std::unique_lock<wpi::mutex> lock(pcm->dataMutex);
  pcm->controlData.bits.closedLoopEnable = value;
  SendControl(pcm.get(), status);
}

HAL_Bool HAL_GetPCMCompressorClosedLoopControl(HAL_PCMHandle handle,
                                               int32_t* status) {
  auto pcmStatus = GetStatus1(handle, status);
  if (*status != 0) return 0;

  return pcmStatus.bits.isCloseloopEnabled != 0;
}

HAL_Bool HAL_GetPCMCompressorPressureSwitch(HAL_PCMHandle handle,
                                            int32_t* status) {
  auto pcmStatus = GetStatus1(handle, status);
  if (*status != 0) return 0;

  return pcmStatus.bits.pressureSwitchEn != 0;
}

double HAL_GetPCMCompressorCurrent(HAL_PCMHandle handle, int32_t* status) {
  auto pcmStatus = GetStatus1(handle, status);
  if (*status != 0) return 0;

  uint32_t temp = pcmStatus.bits.compressorCurrentTop6;
  temp <<= 4;
  temp |= pcmStatus.bits.compressorCurrentBtm4;
  return temp * 0.03125; /* 5.5 fixed pt value in Amps */
}

HAL_Bool HAL_GetPCMCompressorCurrentTooHighFault(HAL_PCMHandle handle,
                                                 int32_t* status) {
  auto pcmStatus = GetStatus1(handle, status);
  if (*status != 0) return 0;

  return pcmStatus.bits.faultCompCurrentTooHigh != 0;
}

HAL_Bool HAL_GetPCMCompressorCurrentTooHighStickyFault(HAL_PCMHandle handle,
                                                       int32_t* status) {
  auto pcmStatus = GetStatus1(handle, status);
  if (*status != 0) return 0;

  return pcmStatus.bits.stickyFaultCompCurrentTooHigh != 0;
}

HAL_Bool HAL_GetPCMCompressorShortedStickyFault(HAL_PCMHandle handle,
                                                int32_t* status) {
  auto pcmStatus = GetStatus1(handle, status);
  if (*status != 0) return 0;

  return pcmStatus.bits.StickyFault_dItooHigh != 0;
}

HAL_Bool HAL_GetPCMCompressorShortedFault(HAL_PCMHandle handle,
                                          int32_t* status) {
  auto pcmStatus = GetStatus1(handle, status);
  if (*status != 0) return 0;

  return pcmStatus.bits.Fault_dItooHigh != 0;
}

HAL_Bool HAL_GetPCMCompressorNotConnectedStickyFault(HAL_PCMHandle handle,
                                                     int32_t* status) {
  auto pcmStatus = GetStatusSolFault(handle, status);
  if (*status != 0) return 0;

  return pcmStatus.bits.StickyFault_CompNoCurrent != 0;
}

HAL_Bool HAL_GetPCMCompressorNotConnectedFault(HAL_PCMHandle handle,
                                               int32_t* status) {
  auto pcmStatus = GetStatusSolFault(handle, status);
  if (*status != 0) return 0;

  return pcmStatus.bits.Fault_CompNoCurrent != 0;
}

int32_t HAL_GetPCMSolenoidBlackList(HAL_PCMHandle handle, int32_t* status) {
  auto pcmStatus = GetStatusSolFault(handle, status);
  if (*status != 0) return 0;

  return pcmStatus.bits.SolenoidBlacklist != 0;
}

HAL_Bool HAL_GetPCMSolenoidVoltageStickyFault(HAL_PCMHandle handle,
                                              int32_t* status) {
  auto pcmStatus = GetStatus1(handle, status);
  if (*status != 0) return 0;

  return pcmStatus.bits.stickyFaultFuseTripped != 0;
}

HAL_Bool HAL_GetPCMSolenoidVoltageFault(HAL_PCMHandle handle, int32_t* status) {
  auto pcmStatus = GetStatus1(handle, status);
  if (*status != 0) return 0;

  return pcmStatus.bits.faultFuseTripped != 0;
}

void HAL_ClearAllPCMStickyFaults(HAL_PCMHandle handle, int32_t* status) {
  auto pcm = pcmHandles->Get(handle);
  if (!pcm) {
    *status = HAL_HANDLE_ERROR;
    return;
  }
  uint8_t pcmSupplemControl[] = {0, 0, 0,
                                 0x80}; /* only bit set is ClearStickyFaults */
  HAL_WriteCANPacket(pcm->handle, pcmSupplemControl, sizeof(pcmSupplemControl),
                     Control2, status);
}

HAL_Bool HAL_GetPCMModuleEnabled(HAL_PCMHandle handle, int32_t* status) {
  auto pcmStatus = GetStatus1(handle, status);
  if (*status != 0) return 0;

  return pcmStatus.bits.moduleEnabled != 0;
}
}  // extern "C"

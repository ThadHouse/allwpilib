#include "hal/PCM.h"

#include <memory>

#include <wpi/mutex.h>

#include "HALInitializer.h"
#include "PortsInternal.h"
#include "hal/CANAPI.h"
#include "hal/Errors.h"
#include "hal/Ports.h"
#include "hal/handles/IndexedHandleResource.h"

using namespace hal;

static constexpr HAL_CANManufacturer manufacturer =
    HAL_CANManufacturer::HAL_CAN_Man_kCTRE;

static constexpr HAL_CANDeviceType deviceType =
    HAL_CANDeviceType::HAL_CAN_Dev_kPneumatics;

static constexpr int32_t DefaultCANPeriod = 20;

static constexpr int32_t Status1 = 0x50;
static constexpr int32_t StatusSolFaults = 0x51;
static constexpr int32_t StatusDebug = 0x52;

static constexpr int32_t Control1 = 0x70;
static constexpr int32_t ControlStickFaults = 0x71;
static constexpr int32_t ControlOneShotDur = 0x72;

static constexpr int32_t TimeoutMs = 100;
static constexpr int32_t StatusPeriodMs = 25;

union PcmStatus1 {
  uint8_t data[8];
  struct Bits {
    /* Byte 0 */
    unsigned SolenoidBits:8;
    /* Byte 1 */
    unsigned compressorOn:1;
    unsigned stickyFaultFuseTripped:1;
    unsigned stickyFaultCompCurrentTooHigh:1;
    unsigned faultFuseTripped:1;
    unsigned faultCompCurrentTooHigh:1;
    unsigned faultHardwareFailure:1;
    unsigned isCloseloopEnabled:1;
    unsigned pressureSwitchEn:1;
    /* Byte 2*/
    unsigned battVoltage:8;
    /* Byte 3 */
    unsigned solenoidVoltageTop8:8;
    /* Byte 4 */
    unsigned compressorCurrentTop6:6;
    unsigned solenoidVoltageBtm2:2;
    /* Byte 5 */
    unsigned StickyFault_dItooHigh :1;
    unsigned Fault_dItooHigh :1;
    unsigned moduleEnabled:1;
    unsigned closedLoopOutput:1;
    unsigned compressorCurrentBtm4:4;
    /* Byte 6 */
    unsigned tokenSeedTop8:8;
    unsigned tokenSeedBtm8:8;
  };
};

union PcmStatusSolFault {
  uint8_t data[8];
  struct Bits {
    /* Byte 0 */
    unsigned SolenoidBlacklist:8;
    /* Byte 1 */
    unsigned reserved_bit0 :1;
    unsigned reserved_bit1 :1;
    unsigned reserved_bit2 :1;
    unsigned reserved_bit3 :1;
    unsigned StickyFault_CompNoCurrent :1;
    unsigned Fault_CompNoCurrent :1;
    unsigned StickyFault_SolenoidJumper :1;
    unsigned Fault_SolenoidJumper :1;
  };
};

union PcmControl1 {
  uint8_t data[8];
  struct Bits {
    /* Byte 0 */
    unsigned tokenTop8:8;
    /* Byte 1 */
    unsigned tokenBtm8:8;
    /* Byte 2 */
    unsigned solenoidBits:8;
    /* Byte 3*/
    unsigned reserved:4;
    unsigned closeLoopOutput:1;
    unsigned compressorOn:1;
    unsigned closedLoopEnable:1;
    unsigned clearStickyFaults:1;
    /* Byte 4 */
    unsigned OneShotField_h8:8;
    /* Byte 5 */
    unsigned OneShotField_l8:8;
  };
};

struct PcmOneShotDuration {
  uint8_t sol10MsPerUnit[8];
};

union PcmDebug {
  uint8_t data[8];
  struct Bits {
    unsigned tokFailsTop8:8;
    unsigned tokFailsBtm8:8;
    unsigned lastFailedTokTop8:8;
    unsigned lastFailedTokBtm8:8;
    unsigned tokSuccessTop8:8;
    unsigned tokSuccessBtm8:8;
  };
};

namespace {
struct PCM {
  HAL_CANHandle canHandle;
  int32_t moduleIndex;
  int32_t refCount;
  HAL_CompressorHandle compressorHandle;
  IndexedHandleResource<HAL_SolenoidHandle, uint8_t, kNumSolenoidChannels, HAL_HandleEnum::Solenoid> solenoidHandles;
};
}

static wpi::mutex pcmHandleMutex;
static IndexedHandleResource<HAL_PCMHandle, PCM, kNumPCMModules, HAL_HandleEnum::PCM>* pcmHandles;
static HAL_PCMHandle pcmHandleIndexMap[kNumPCMModules];

namespace hal {
namespace init {
void InitializePCM() {
  static IndexedHandleResource<HAL_PCMHandle, PCM, kNumPCMModules, HAL_HandleEnum::PCM> pH;
  pcmHandles = &pH;
  for (int i = 0; i < kNumPCMModules; i++) {
    pcmHandleIndexMap[i] = HAL_kInvalidHandle;
  }
}
}  // namespace init
}  // namespace hal

extern "C" {

HAL_PCMHandle HAL_InitializePCM(int32_t module, int32_t* status) {
  hal::init::CheckInit();
  if (!HAL_CheckPCMModule(module)) {
    *status = PARAMETER_OUT_OF_RANGE;
    return HAL_kInvalidHandle;
  }

  std::lock_guard<wpi::mutex> lock(pcmHandleMutex);

  if (pcmHandleIndexMap[module] != HAL_kInvalidHandle) {
    auto existingHandle = pcmHandleIndexMap[module];
    auto existingData = pcmHandles->Get(existingHandle);
    if (existingData) {
      *status = 0;
      existingData->refCount++;
      return existingHandle;
    }
    // If not existing data, fall through, weird allocation
    pcmHandleIndexMap[module] = HAL_kInvalidHandle;
  }

  auto canHandle = HAL_InitializeCAN(manufacturer, module, deviceType, status);

  if (*status != 0) {
    HAL_CleanCAN(canHandle);
    return HAL_kInvalidHandle;
  }

  auto handle = pcmHandles->Allocate(module, status);

  if (*status != 0) {
    // Failed to allocate
    HAL_CleanCAN(canHandle);
    return HAL_kInvalidHandle;
  }

  auto data = pcmHandles->Get(handle);

  if (data == nullptr) {  // would only occur on thread issue.
    *status = HAL_HANDLE_ERROR;
    return HAL_kInvalidHandle;
  }

  data->moduleIndex = module;

  data->refCount = 1;
  data->canHandle = canHandle;

  return handle;
}

HAL_Bool HAL_CleanPCM(HAL_PCMHandle handle) {
  std::lock_guard<wpi::mutex> lock(pcmHandleMutex);

  auto data = pcmHandles->Get(handle);
  if (data == nullptr) {
    return false;
  }

  data->refCount--;
  if (data->refCount == 0) {
    // Last one
    HAL_CleanCAN(data->canHandle);
    pcmHandleIndexMap[data->moduleIndex] = HAL_kInvalidHandle;
    pcmHandles->Free(handle);
    return true;
  }

  // Not last one, just return false
  return false;
}

}

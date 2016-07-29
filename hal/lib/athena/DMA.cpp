/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2016. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "HAL/DMA.h"

#include <algorithm>
#include <memory>
#include <type_traits>

#include "AnalogInternal.h"
#include "ChipObject.h"
#include "DigitalInternal.h"
#include "EncoderInternal.h"
#include "HAL/AnalogInput.h"
#include "HAL/Encoder.h"
#include "HAL/Errors.h"
#include "HAL/handles/HandlesInternal.h"
#include "HAL/handles/UnlimitedHandleResource.h"
#include "PortsInternal.h"

using namespace hal;

namespace {
struct DMA {
  std::unique_ptr<tDMAManager> manager;
  std::unique_ptr<tDMA> aDMA;

  // The offsets into the sample structure for each DMA type, or -1 if it isn't
  // in the set of values.
  ssize_t channel_offsets[20];

  std::array<bool, 8> trigger_channels = {
      {false, false, false, false, false, false, false, false}};
  size_t capture_size;
};

struct DMASample {
  uint32_t read_buffer[64];
};
}  // namespace

static constexpr ssize_t kChannelSize[20] = {2, 2, 4, 4, 2, 2, 4, 4, 3, 3,
                                             2, 1, 4, 4, 4, 4, 4, 4, 4, 4};

enum DMAOffsetConstants {
  kEnable_AI0_Low = 0,
  kEnable_AI0_High = 1,
  kEnable_AIAveraged0_Low = 2,
  kEnable_AIAveraged0_High = 3,
  kEnable_AI1_Low = 4,
  kEnable_AI1_High = 5,
  kEnable_AIAveraged1_Low = 6,
  kEnable_AIAveraged1_High = 7,
  kEnable_Accumulator0 = 8,
  kEnable_Accumulator1 = 9,
  kEnable_DI = 10,
  kEnable_AnalogTriggers = 11,
  kEnable_Counters_Low = 12,
  kEnable_Counters_High = 13,
  kEnable_CounterTimers_Low = 14,
  kEnable_CounterTimers_High = 15,
  kEnable_Encoders_Low = 16,
  kEnable_Encoders_High = 17,
  kEnable_EncoderTimers_Low = 18,
  kEnable_EncoderTimers_High = 19,
};

static UnlimitedHandleResource<HAL_DMASampleHandle, DMASample,
                               HAL_HandleEnum::DMASample>
    dmaSampleHandles;
static std::unique_ptr<DMA> dmaPointer;

extern "C" {

void HAL_InitializeDMA(int32_t* status) {
  // Only 1 DMA pointer allowed, so we do not have to have it in a list
  dmaPointer = std::make_unique<DMA>();

  // Manager does not get created until DMA is started
  dmaPointer->aDMA.reset(tDMA::create(status));
  if (status != 0) {
    return;
  }
  dmaPointer->aDMA->writeConfig_ExternalClock(false, status);
  if (status != 0) {
    return;
  }

  HAL_SetDMARate(1, status);
  if (status != 0) {
    return;
  }
  HAL_SetDMAPause(false, status);
  if (status != 0) {
    return;
  }
}
void HAL_FreeDMA(void) {
  if (dmaPointer == nullptr) return;

  int32_t status = 0;
  if (dmaPointer->manager != nullptr) {
    dmaPointer->manager->stop(&status);
  }
  dmaPointer = nullptr;
}

void HAL_SetDMAPause(HAL_Bool pause, int32_t* status) {
  if (!dmaPointer) {
    *status = INCOMPATIBLE_STATE;
    return;
  }

  dmaPointer->aDMA->writeConfig_Pause(pause, status);
}
void HAL_SetDMARate(int32_t cycles, int32_t* status) {
  if (dmaPointer == nullptr) {
    *status = INCOMPATIBLE_STATE;
    return;
  }

  if (cycles < 1) {
    cycles = 1;
  }

  dmaPointer->aDMA->writeRate(cycles, status);
}

void HAL_AddDMAEncoder(HAL_EncoderHandle encoderHandle, int32_t* status) {
  // Detect a counter encoder vs an actual encoder, and use the right DMA calls
  HAL_FPGAEncoderHandle fpgaEncoderHandle = 0;
  HAL_CounterHandle counterHandle = 0;

  bool validEncoderHandle =
      GetEncoderHandles(encoderHandle, &fpgaEncoderHandle, &counterHandle);
  if (!validEncoderHandle) {
    *status = HAL_HANDLE_ERROR;
    return;
  }

  if (counterHandle != HAL_kInvalidHandle) {
    HAL_AddDMACounter(counterHandle, status);
    return;
  }

  if (dmaPointer == nullptr) {
    *status = INCOMPATIBLE_STATE;
    return;
  }

  if (dmaPointer->manager) {
    *status = HAL_INVALID_DMA_ADDITION;
    return;
  }

  int32_t index =
      getHandleTypedIndex(fpgaEncoderHandle, HAL_HandleEnum::FPGAEncoder);
  if (index < 0) {
    *status = HAL_HANDLE_ERROR;
    return;
  }

  if (index < 4) {
    dmaPointer->aDMA->writeConfig_Enable_Encoders_Low(true, status);
  } else if (index < 8) {
    dmaPointer->aDMA->writeConfig_Enable_Encoders_High(true, status);
  } else {
    *status = NiFpga_Status_InvalidParameter;
  }
}
void HAL_AddDMACounter(HAL_CounterHandle counterHandle, int32_t* status) {
  if (dmaPointer == nullptr) {
    *status = INCOMPATIBLE_STATE;
    return;
  }

  if (dmaPointer->manager) {
    *status = HAL_INVALID_DMA_ADDITION;
    return;
  }

  int32_t index = getHandleTypedIndex(counterHandle, HAL_HandleEnum::Counter);
  if (index < 0) {
    *status = HAL_HANDLE_ERROR;
    return;
  }

  if (index < 4) {
    dmaPointer->aDMA->writeConfig_Enable_Counters_Low(true, status);
  } else if (index < 8) {
    dmaPointer->aDMA->writeConfig_Enable_Counters_High(true, status);
  } else {
    *status = NiFpga_Status_InvalidParameter;
  }
}
void HAL_AddDMADigitalSource(HAL_Handle digitalSourceHandle, int32_t* status) {
  if (dmaPointer == nullptr) {
    *status = INCOMPATIBLE_STATE;
    return;
  }

  if (dmaPointer->manager) {
    *status = HAL_INVALID_DMA_ADDITION;
    return;
  }
  // Check for an AnalogTrigger, and enable that is requested
  if (isHandleType(digitalSourceHandle, HAL_HandleEnum::AnalogTrigger)) {
    dmaPointer->aDMA->writeConfig_Enable_AnalogTriggers(true, status);
  } else if (isHandleType(digitalSourceHandle, HAL_HandleEnum::DIO)) {
    dmaPointer->aDMA->writeConfig_Enable_DI(true, status);
  } else {
    *status = NiFpga_Status_InvalidParameter;
  }
}
void HAL_AddDMAAnalogInput(HAL_AnalogInputHandle aInHandle, int32_t* status) {
  if (dmaPointer == nullptr) {
    *status = INCOMPATIBLE_STATE;
    return;
  }

  if (dmaPointer->manager) {
    *status = HAL_INVALID_DMA_ADDITION;
    return;
  }

  int32_t channel = getHandleTypedIndex(aInHandle, HAL_HandleEnum::AnalogInput);
  if (channel < 0) {
    *status = HAL_HANDLE_ERROR;
    return;
  }

  if (channel <= 3) {
    dmaPointer->aDMA->writeConfig_Enable_AI0_Low(true, status);
  } else {
    dmaPointer->aDMA->writeConfig_Enable_AI0_High(true, status);
  }
}

void HAL_SetDMAExternalTrigger(HAL_Handle digitalSourceHandle,
                               HAL_AnalogTriggerType analogTriggerType,
                               HAL_Bool rising, HAL_Bool falling,
                               int32_t* status) {
  if (dmaPointer == nullptr) {
    *status = INCOMPATIBLE_STATE;
    return;
  }

  if (dmaPointer->manager) {
    *status = HAL_INVALID_DMA_ADDITION;
    return;
  }

  auto index = std::find(dmaPointer->trigger_channels.begin(),
                         dmaPointer->trigger_channels.end(), false);
  if (index == dmaPointer->trigger_channels.end()) {
    *status = NO_AVAILABLE_RESOURCES;
    return;
  }

  *index = true;

  int channelIndex = std::distance(dmaPointer->trigger_channels.begin(), index);

  bool isExternalClock = dmaPointer->aDMA->readConfig_ExternalClock(status);
  if (*status == 0) {
    if (!isExternalClock) {
      dmaPointer->aDMA->writeConfig_ExternalClock(true, status);
      if (*status != 0) {
        return;
      }
    }
  } else {
    return;
  }

  uint8_t pin = 0;
  uint8_t module = 0;
  bool analogTrigger = false;
  bool success = remapDigitalSource(digitalSourceHandle, analogTriggerType, pin,
                                    module, analogTrigger);
  if (!success) {
    *status = PARAMETER_OUT_OF_RANGE;
    return;
  }

  tDMA::tExternalTriggers newTrigger;

  newTrigger.FallingEdge = falling;
  newTrigger.RisingEdge = rising;
  newTrigger.ExternalClockSource_AnalogTrigger = analogTrigger;
  newTrigger.ExternalClockSource_Channel = pin;
  newTrigger.ExternalClockSource_Module = module;

  // Configures the trigger to be external, not off the FPGA clock.
  dmaPointer->aDMA->writeExternalTriggers(channelIndex / 4, channelIndex % 4,
                                          newTrigger, status);
}

void HAL_StartDMA(int32_t queueDepth, int32_t* status) {
  if (dmaPointer == nullptr) {
    *status = INCOMPATIBLE_STATE;
    return;
  }

  tDMA::tConfig config = dmaPointer->aDMA->readConfig(status);
  if (*status != 0) return;

  {
    size_t accum_size = 0;
#define SET_SIZE(bit)                                 \
  if (config.bit) {                                   \
    dmaPointer->channel_offsets[k##bit] = accum_size; \
    accum_size += kChannelSize[k##bit];               \
  } else {                                            \
    dmaPointer->channel_offsets[k##bit] = -1;         \
  }

    SET_SIZE(Enable_AI0_Low);
    SET_SIZE(Enable_AI0_High);
    SET_SIZE(Enable_AIAveraged0_Low);
    SET_SIZE(Enable_AIAveraged0_High);
    SET_SIZE(Enable_AI1_Low);
    SET_SIZE(Enable_AI1_High);
    SET_SIZE(Enable_AIAveraged1_Low);
    SET_SIZE(Enable_AIAveraged1_High);
    SET_SIZE(Enable_Accumulator0);
    SET_SIZE(Enable_Accumulator1);
    SET_SIZE(Enable_DI);
    SET_SIZE(Enable_AnalogTriggers);
    SET_SIZE(Enable_Counters_Low);
    SET_SIZE(Enable_Counters_High);
    SET_SIZE(Enable_CounterTimers_Low);
    SET_SIZE(Enable_CounterTimers_High);
    SET_SIZE(Enable_Encoders_Low);
    SET_SIZE(Enable_Encoders_High);
    SET_SIZE(Enable_EncoderTimers_Low);
    SET_SIZE(Enable_EncoderTimers_High);
#undef SET_SIZE
    dmaPointer->capture_size = accum_size + 1;
  }

  dmaPointer->manager = std::make_unique<tDMAManager>(
      0, queueDepth * dmaPointer->capture_size, status);
  if (*status != 0) return;

  // Start, stop, start to clear the buffer.
  dmaPointer->manager->start(status);
  if (*status != 0) return;
  dmaPointer->manager->stop(status);
  if (*status != 0) return;
  dmaPointer->manager->start(status);
  if (*status != 0) return;
}

enum HAL_DMAReadStatus HAL_ReadDMA(HAL_DMASampleHandle dmaSampleHandle,
                                   int32_t timeoutMs, int32_t* remainingOut,
                                   int32_t* status) {
  auto dmaSample = dmaSampleHandles.Get(dmaSampleHandle);
  if (dmaSample == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return HAL_DMA_Error;
  }

  *remainingOut = 0;
  size_t remainingBytes = 0;

  if (dmaPointer->manager == nullptr) {
    *status = NiFpga_Status_InvalidParameter;
    return HAL_DMA_Error;
  }

  dmaPointer->manager->read(dmaSample->read_buffer, dmaPointer->capture_size,
                            timeoutMs, &remainingBytes, status);

  *remainingOut = remainingBytes / dmaPointer->capture_size;

  if (*status == 0) {
    return HAL_DMA_OK;
  } else if (*status == NiFpga_Status_FifoTimeout) {
    // Don't status error on timeout
    *status = 0;
    return HAL_DMA_Timeout;
  } else {
    return HAL_DMA_Error;
  }
}

static int GetDMAOffset(DMA* dma, int index) {
  return dma->channel_offsets[index];
}

// Sampling Code
double HAL_GetDMASampleTimestamp(HAL_DMASampleHandle dmaSampleHandle,
                                 int32_t* status) {
  return static_cast<double>(HAL_GetDMASampleTime(dmaSampleHandle, status)) *
         0.000001;
}
int32_t HAL_GetDMASampleTime(HAL_DMASampleHandle dmaSampleHandle,
                             int32_t* status) {
  auto dmaSample = dmaSampleHandles.Get(dmaSampleHandle);
  if (dmaSample == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return 0;
  }

  return dmaSample->read_buffer[dmaPointer->capture_size - 1];
}

int32_t HAL_GetDMASampleEncoder(HAL_DMASampleHandle dmaSampleHandle,
                                HAL_EncoderHandle encoderHandle,
                                int32_t* status) {
  auto scale = HAL_GetEncoderEncodingScale(encoderHandle, status);
  if (*status != 0) {
    return 0;
  }

  return HAL_GetDMASampleEncoderRaw(dmaSampleHandle, encoderHandle, status) /
         scale;
}
int32_t HAL_GetDMASampleEncoderRaw(HAL_DMASampleHandle dmaSampleHandle,
                                   HAL_EncoderHandle encoderHandle,
                                   int32_t* status) {
  HAL_FPGAEncoderHandle fpgaEncoderHandle = 0;
  HAL_CounterHandle counterHandle = 0;

  bool validEncoderHandle =
      GetEncoderHandles(encoderHandle, &fpgaEncoderHandle, &counterHandle);
  if (!validEncoderHandle) {
    *status = HAL_HANDLE_ERROR;
    return -1;
  }

  if (counterHandle != HAL_kInvalidHandle) {
    return HAL_GetDMASampleCounter(dmaSampleHandle, counterHandle, status);
  }

  auto dmaSample = dmaSampleHandles.Get(dmaSampleHandle);
  if (dmaSample == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return -1;
  }

  DMA* dmaRawPtr = dmaPointer.get();

  int32_t index =
      getHandleTypedIndex(fpgaEncoderHandle, HAL_HandleEnum::FPGAEncoder);
  if (index < 0) {
    *status = PARAMETER_OUT_OF_RANGE;
    return -1;
  }

  uint32_t dmaWord = 0;

  if (index < 4) {
    if (GetDMAOffset(dmaRawPtr, kEnable_Encoders_Low) == -1) {
      *status = NiFpga_Status_ResourceNotFound;
      return -1;
    }
    dmaWord =
        dmaSample->read_buffer[GetDMAOffset(dmaRawPtr, kEnable_Encoders_Low) +
                               index];
  } else if (index < 8) {
    if (GetDMAOffset(dmaRawPtr, kEnable_Encoders_High) == -1) {
      *status = NiFpga_Status_ResourceNotFound;
      return -1;
    }
    dmaWord =
        dmaSample->read_buffer[GetDMAOffset(dmaRawPtr, kEnable_Encoders_High) +
                               (index - 4)];
  } else {
    *status = NiFpga_Status_ResourceNotFound;
    return 0;
  }

  int32_t result = dmaWord;

  result = result << 1;

  return result;
}
int32_t HAL_GetDMASampleCounter(HAL_DMASampleHandle dmaSampleHandle,
                                HAL_CounterHandle counterHandle,
                                int32_t* status) {
  int32_t index = getHandleTypedIndex(counterHandle, HAL_HandleEnum::Counter);
  if (index < 0) {
    *status = HAL_HANDLE_ERROR;
    return -1;
  }
  auto dmaSample = dmaSampleHandles.Get(dmaSampleHandle);
  if (dmaSample == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return -1;
  }

  DMA* dmaRawPtr = dmaPointer.get();

  uint32_t dmaWord = 0;

  if (index < 4) {
    if (GetDMAOffset(dmaRawPtr, kEnable_Counters_Low) == -1) {
      *status = NiFpga_Status_ResourceNotFound;
      return -1;
    }
    dmaWord =
        dmaSample->read_buffer[GetDMAOffset(dmaRawPtr, kEnable_Counters_Low) +
                               index];
  } else if (index < 8) {
    if (GetDMAOffset(dmaRawPtr, kEnable_Counters_High) == -1) {
      *status = NiFpga_Status_ResourceNotFound;
      return -1;
    }
    dmaWord =
        dmaSample->read_buffer[GetDMAOffset(dmaRawPtr, kEnable_Counters_High) +
                               (index - 4)];
  } else {
    *status = NiFpga_Status_ResourceNotFound;
    return 0;
  }

  int32_t result = dmaWord;

  result = result << 1;

  return result;
}
HAL_Bool HAL_GetDMASampleDigitalSource(HAL_DMASampleHandle dmaSampleHandle,
                                       HAL_Handle dSourceHandle,
                                       int32_t* status) {
  auto dmaSample = dmaSampleHandles.Get(dmaSampleHandle);
  if (dmaSample == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return -1;
  }

  DMA* dmaRawPtr = dmaPointer.get();

  HAL_HandleEnum handleType = getHandleType(dSourceHandle);
  int32_t index = getHandleIndex(dSourceHandle);

  if (handleType == HAL_HandleEnum::DIO) {
    if (GetDMAOffset(dmaRawPtr, kEnable_DI) == -1) {
      *status = NiFpga_Status_ResourceNotFound;
      return false;
    }
    if (index < kNumDigitalHeaders) {
      return (dmaSample->read_buffer[GetDMAOffset(dmaRawPtr, kEnable_DI)] >>
              index) &
             0x1;
    } else {
      return (dmaSample->read_buffer[GetDMAOffset(dmaRawPtr, kEnable_DI)] >>
              (index + 6)) &
             0x1;
    }
  } else if (handleType == HAL_HandleEnum::AnalogTrigger) {
    if (GetDMAOffset(dmaRawPtr, kEnable_AnalogTriggers) == -1) {
      *status = NiFpga_Status_ResourceNotFound;
      return false;
    }
    return (dmaSample->read_buffer[GetDMAOffset(dmaRawPtr,
                                                kEnable_AnalogTriggers)] >>
            index) &
           0x1;
  } else {
    *status = NiFpga_Status_InvalidParameter;
    return false;
  }
}
int32_t HAL_GetDMASampleAnalogInputRaw(HAL_DMASampleHandle dmaSampleHandle,
                                       HAL_AnalogInputHandle aInHandle,
                                       int32_t* status) {
  auto dmaSample = dmaSampleHandles.Get(dmaSampleHandle);
  if (dmaSample == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return -1;
  }

  DMA* dmaRawPtr = dmaPointer.get();

  int32_t index = getHandleTypedIndex(aInHandle, HAL_HandleEnum::AnalogInput);
  if (index < 0) {
    *status = PARAMETER_OUT_OF_RANGE;
    return 0xffff;
  }
  uint32_t dmaWord = 0;
  if (index < 4) {
    if (GetDMAOffset(dmaRawPtr, kEnable_AI0_Low) == -1) {
      *status = NiFpga_Status_ResourceNotFound;
      return 0xffff;
    }
    dmaWord =
        dmaSample
            ->read_buffer[GetDMAOffset(dmaRawPtr, kEnable_AI0_Low) + index / 2];
  } else if (index < 8) {
    if (GetDMAOffset(dmaRawPtr, kEnable_AI0_High) == -1) {
      *status = NiFpga_Status_ResourceNotFound;
      return 0xffff;
    }
    dmaWord = dmaSample->read_buffer[GetDMAOffset(dmaRawPtr, kEnable_AI0_High) +
                                     (index - 4) / 2];
  } else {
    *status = NiFpga_Status_ResourceNotFound;
    return 0xffff;
  }

  if (index % 2) {
    return (dmaWord >> 16) & 0xffff;
  } else {
    return dmaWord & 0xffff;
  }
}
double HAL_GetDMASampleAnalogInputVoltage(HAL_DMASampleHandle dmaSampleHandle,
                                          HAL_AnalogInputHandle aInHandle,
                                          int32_t* status) {
  int32_t value =
      HAL_GetDMASampleAnalogInputRaw(dmaSampleHandle, aInHandle, status);
  if (*status != 0) {
    return 0.0;
  }
  if (value == 0xffff) return 0.0;
  int32_t lsbWeight = HAL_GetAnalogLSBWeight(aInHandle, status);
  if (*status != 0) return 0.0;
  int32_t offset = HAL_GetAnalogOffset(aInHandle, status);
  if (*status != 0) return 0.0;
  double voltage = lsbWeight * 1.0e-9 * value - offset * 1.0e-9;
  return voltage;
}
}  // extern "C"

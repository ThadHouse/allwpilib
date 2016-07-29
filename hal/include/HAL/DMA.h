/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2016. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#pragma once

#include <stdint.h>

#include "HAL/AnalogTrigger.h"
#include "HAL/Types.h"

enum HAL_DMAReadStatus {
  HAL_DMA_OK = 1,
  HAL_DMA_Timeout = 2,
  HAL_DMA_Error = 3
};

#ifdef __cplusplus
extern "C" {
#endif

void HAL_InitializeDMA(int32_t* status);
void HAL_FreeDMA(void);

void HAL_SetDMAPause(HAL_Bool pause, int32_t* status);
void HAL_SetDMARate(int32_t cycles, int32_t* status);

void HAL_AddDMAEncoder(HAL_EncoderHandle encoderHandle, int32_t* status);
void HAL_AddDMACounter(HAL_CounterHandle counterHandle, int32_t* status);
void HAL_AddDMADigitalSource(HAL_Handle digitalSourceHandle, int32_t* status);
void HAL_AddDMAAnalogInput(HAL_AnalogInputHandle aInHandle, int32_t* status);

void HAL_SetDMAExternalTrigger(HAL_Handle digitalSourceHandle,
                               HAL_AnalogTriggerType analogTriggerType,
                               HAL_Bool rising, HAL_Bool falling,
                               int32_t* status);

void HAL_StartDMA(int32_t queueDepth, int32_t* status);

HAL_DMASampleHandle HAL_MakeDMASample(int32_t* status);
void HAL_FreeDMASample(HAL_DMASampleHandle handle);

enum HAL_DMAReadStatus HAL_ReadDMA(HAL_DMASampleHandle dmaSampleHandle,
                                   int32_t timeoutMs, int32_t* remainingOut,
                                   int32_t* status);

// Sampling Code
double HAL_GetDMASampleTimestamp(HAL_DMASampleHandle dmaSampleHandle,
                                 int32_t* status);
int32_t HAL_GetDMASampleTime(HAL_DMASampleHandle dmaSampleHandle,
                             int32_t* status);

int32_t HAL_GetDMASampleEncoder(HAL_DMASampleHandle dmaSampleHandle,
                                HAL_EncoderHandle encoderHandle,
                                int32_t* status);
int32_t HAL_GetDMASampleEncoderRaw(HAL_DMASampleHandle dmaSampleHandle,
                                   HAL_EncoderHandle encoderHandle,
                                   int32_t* status);
int32_t HAL_GetDMASampleCounter(HAL_DMASampleHandle dmaSampleHandle,
                                HAL_CounterHandle counterHandle,
                                int32_t* status);
HAL_Bool HAL_GetDMASampleDigitalSource(HAL_DMASampleHandle dmaSampleHandle,
                                       HAL_Handle dSourceHandle,
                                       int32_t* status);
int32_t HAL_GetDMASampleAnalogInputRaw(HAL_DMASampleHandle dmaSampleHandle,
                                       HAL_AnalogInputHandle aInHandle,
                                       int32_t* status);
double HAL_GetDMASampleAnalogInputVoltage(HAL_DMASampleHandle dmaSampleHandle,
                                          HAL_AnalogInputHandle aInHandle,
                                          int32_t* status);

#ifdef __cplusplus
}  // extern "C"
#endif

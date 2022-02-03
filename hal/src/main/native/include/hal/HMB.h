#pragma once

#include "hal/Types.h"

#ifdef __cplusplus
extern "C" {
#endif

HAL_HMBHandle HAL_InitializeHMB(int32_t* status);

void HAL_FreeHMB(HAL_HMBHandle handle);

void HAL_StrobeHMB(HAL_HMBHandle handle, int32_t* status);

uint64_t HAL_ReadHMBFPGATimestamp(HAL_HMBHandle handle, int32_t* status);

uint32_t HAL_ReadHMBDIO(HAL_HMBHandle handle, int32_t* status);

uint32_t* HAL_GetHMBBuffer(HAL_HMBHandle handle, int32_t* status);

#ifdef __cplusplus
}  // extern "C"
#endif

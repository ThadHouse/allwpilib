/*----------------------------------------------------------------------------*/
/* Copyright (c) 2018 FIRST. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#pragma once

#include <stdint.h>

#include "HAL/Types.h"

#ifdef __cplusplus
extern "C" {
#endif

HAL_PCMHandle HAL_InitializePCM(int32_t module, int32_t* status);
void HAL_FreePCM(HAL_PCMHandle handle);
HAL_Bool HAL_CheckPCMModule(int32_t channel);
HAL_Bool HAL_CheckSolenoidChannel(int32_t channel);
HAL_Bool HAL_GetSolenoid(HAL_PCMHandle solenoidPortHandle, int32_t* status);
int32_t HAL_GetAllSolenoids(int32_t module, int32_t* status);
void HAL_SetSolenoid(HAL_PCMHandle solenoidPortHandle, HAL_Bool value,
                     int32_t* status);
void HAL_SetAllSolenoids(int32_t module, int32_t state, int32_t* status);
int32_t HAL_GetPCMSolenoidBlackList(int32_t module, int32_t* status);
HAL_Bool HAL_GetPCMSolenoidVoltageStickyFault(int32_t module, int32_t* status);
HAL_Bool HAL_GetPCMSolenoidVoltageFault(int32_t module, int32_t* status);
void HAL_ClearAllPCMStickyFaults(int32_t module, int32_t* status);
void HAL_SetOneShotDuration(HAL_PCMHandle solenoidPortHandle, int32_t durMS,
                            int32_t* status);
void HAL_FireOneShot(HAL_PCMHandle solenoidPortHandle, int32_t* status);

#ifdef __cplusplus
}  // extern "C"
#endif

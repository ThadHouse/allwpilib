/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2016-2017. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#pragma once

#include <stdint.h>

#include "HAL/Types.h"

enum HAL_PowerStatus : int32_t {
  HAL_PowerStatus_Disable = 0,
  HAL_PowerStatus_Brownout = 1,
  HAL_PowerStatus_Overcurrent = 2,
  HAL_PowerStatus_VoltageOutOfRange = 3,
  HAL_PowerStatus_PowerGood = 4
};

#ifdef __cplusplus
extern "C" {
#endif

double HAL_GetVinVoltage(int32_t* status);
double HAL_GetVinCurrent(int32_t* status);
double HAL_GetUserVoltage6V(int32_t* status);
double HAL_GetUserCurrent6V(int32_t* status);
HAL_PowerStatus HAL_GetUserStatus6V(int32_t* status);
int32_t HAL_GetUserCurrentFaults6V(int32_t* status);
void HAL_SetUserVoltage6VDisabled(HAL_Bool disable, int32_t* status);
double HAL_GetUserVoltage5V(int32_t* status);
double HAL_GetUserCurrent5V(int32_t* status);
HAL_PowerStatus HAL_GetUserStatus5V(int32_t* status);
int32_t HAL_GetUserCurrentFaults5V(int32_t* status);
void HAL_SetUserVoltage5VDisabled(HAL_Bool disable, int32_t* status);
double HAL_GetUserVoltage3V3(int32_t* status);
double HAL_GetUserCurrent3V3(int32_t* status);
HAL_PowerStatus HAL_GetUserStatus3V3(int32_t* status);
int32_t HAL_GetUserCurrentFaults3V3(int32_t* status);
void HAL_SetUserVoltage3V3Disabled(HAL_Bool disable, int32_t* status);

#ifdef __cplusplus
}
#endif

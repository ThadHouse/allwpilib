/*----------------------------------------------------------------------------*/
/* Copyright (c) 2016-2018 FIRST. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#pragma once

#include <stdint.h>

#include "hal/Types.h"

/**
 * @defgroup hal_solenoid Solenoid Output Functions
 * @ingroup hal_capi
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initializes a solenoid port.
 *
 * @param portHandle the port handle of the module and channel to initialize
 * @return           the created solenoid handle
 */
HAL_SolenoidHandle HAL_InitializeSolenoidPort(HAL_PortHandle portHandle,
                                              int32_t* status);

/**
 * Initializes a solenoid port.
 *
 * @param portHandle the port handle of the module and channel to initialize
 * @return           the created solenoid handle
 */
HAL_SolenoidHandle HAL_InitializeSolenoidPortFromPCM(HAL_PCMHandle pcm, int32_t port,
                                              int32_t* status);

/**
 * Frees a solenoid port.
 *
 * @param solenoidPortHandle the solenoid handle
 */
void HAL_FreeSolenoidPort(HAL_SolenoidHandle solenoidPortHandle);

/**
 * Checks if a solenoid module is in the valid range.
 *
 * @param module the module number to check
 * @return       true if the module number is valid, otherwise false
 */
HAL_Bool HAL_CheckSolenoidModule(int32_t module);

/**
 * Checks if a solenoid channel is in the valid range.
 *
 * @param channel the channel number to check
 * @return       true if the channel number is valid, otherwise false
 */
HAL_Bool HAL_CheckSolenoidChannel(int32_t channel);

/**
 * Gets the current solenoid output value.
 *
 * @param solenoidPortHandle the solenoid handle
 * @return                   true if the solenoid is on, otherwise false
 */
HAL_Bool HAL_GetSolenoid(HAL_SolenoidHandle solenoidPortHandle,
                         int32_t* status);

/**
 * Sets a solenoid output value.
 *
 * @param solenoidPortHandle the solenoid handle
 * @param value              true for on, false for off
 */
void HAL_SetSolenoid(HAL_SolenoidHandle solenoidPortHandle, HAL_Bool value,
                     int32_t* status);

/**
 * Sets the one shot duration on a solenoid channel.
 *
 * @param solenoidPortHandle the solenoid handle
 * @param durMS              the one shot duration in ms
 */
void HAL_SetOneShotDuration(HAL_SolenoidHandle solenoidPortHandle,
                            int32_t durMS, int32_t* status);

/**
 * Fires a single pulse on a solenoid channel.
 *
 * The pulse is the duration set by HAL_SetOneShotDuration().
 *
 * @param solenoidPortHandle the solenoid handle
 */
void HAL_FireOneShot(HAL_SolenoidHandle solenoidPortHandle, int32_t* status);
#ifdef __cplusplus
}  // extern "C"
#endif
/** @} */

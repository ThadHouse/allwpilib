/*----------------------------------------------------------------------------*/
/* Copyright (c) 2018 FIRST. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#pragma once

#include <stdint.h>

#include "hal/Types.h"

/**
 * @defgroup hal_pcm PCM Functions
 * @ingroup hal_capi
 * Functions to control the Pneumatics Control Module.
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initializes a Pneumatics Control Module handle.
 *
 * If the module has not been created, it will be created. Otherwise, the ref
 * count for the existing module will just be increased
 *
 * @param  module the module number to initialize
 * @return the created PCM handle
 */
HAL_PCMHandle HAL_GetPCMHandle(int32_t module, int32_t* status);

/**
 * Cleans a PCM module.
 *
 * The object will only be cleaned when the last handle is returned.
 *
 * @param handle the module handle
 */
void HAL_FreePCMHandle(HAL_PCMHandle handle);

/**
 * Checks if a PCM channel is valid.
 *
 * @param channel the channel to check
 * @return        true if the channel is valid, otherwise false
 */
HAL_Bool HAL_CheckPCMChannel(int32_t channel);

/**
 * Checks if a PCM module is valid.
 *
 * @param channel the module to check
 * @return        true if the module is valid, otherwise false
 */
HAL_Bool HAL_CheckPCMModule(int32_t module);

/**
 * Gets the PCM input voltage.
 *
 * @param handle the module handle
 * @return       the input voltage (volts)
 */
double HAL_GetPCMInputVoltage(HAL_PCMHandle handle, int32_t* status);

/**
 * Gets the PCM solenoid voltage.
 *
 * @param handle the module handle
 * @return       the solenoid voltage (volts)
 */
double HAL_GetPCMSolenoidVoltage(HAL_PCMHandle handle, int32_t* status);

/**
 * Gets the current solenoid output value.
 *
 * @param solenoidPortHandle the solenoid handle
 * @return                   true if the solenoid is on, otherwise false
 */
HAL_Bool HAL_GetPCMSolenoid(HAL_PCMHandle handle, int32_t index, int32_t* status);

/**
 * Gets the status of all solenoids on a specific module.
 *
 * @param module the module to check
 * @return       bitmask of the channels, 1 for on 0 for off
 */
int32_t HAL_GetPCMAllSolenoids(HAL_PCMHandle handle, int32_t* status);

/**
 * Sets a solenoid output value.
 *
 * @param handle the module handle
 * @param index  the solenoid index
 * @param value  true for on, false for off
 */
void HAL_SetPCMSolenoid(HAL_PCMHandle handle, int32_t index, HAL_Bool value,
                        int32_t* status);

/**
 * Sets all solenoid outputs in a module.
 *
 * Only the bottom 8 bits of the value parameter are used. 1 is true, 0 is
 * false. Bits 0-7 are ports 0-7.
 *
 * @param handle the module handle
 * @param values the values to set
 */
void HAL_SetPCMAllSolenoids(HAL_PCMHandle handle, int32_t values,
                            int32_t* status);

/**
 * Sets the one shot duration on a solenoid channel.
 *
 * @param handle the module handle
 * @param index  the solenoid index
 * @param durMS  the one shot duration in ms
 */
void HAL_SetPCMOneShotDuration(HAL_PCMHandle handle, int32_t index,
                               int32_t durMS, int32_t* status);

/**
 * Fires a single pulse on a solenoid channel.
 *
 * The pulse is the duration set by HAL_SetPCMOneShotDuration().
 *
 * @param handle the module handle
 * @param index  the solenoid index
 */
void HAL_FirePCMOneShot(HAL_PCMHandle handle, int32_t index, int32_t* status);

/**
 * Gets the compressor state (on or off).
 *
 * @param  handle the handle
 * @return true if the compressor is on, otherwise false
 */
HAL_Bool HAL_GetPCMCompressorOn(HAL_PCMHandle handle, int32_t* status);

/**
 * Sets the compressor to closed loop mode.
 *
 * @param handle the module handle
 * @param value  true for closed loop mode, false for off
 */
void HAL_SetPCMCompressorClosedLoopControl(HAL_PCMHandle handle, HAL_Bool value,
                                           int32_t* status);

/**
 * Gets if the compressor is in closed loop mode.
 *
 * @param  handle the module handle
 * @return true if the compressor is in closed loop mode, otherwise false
 */
HAL_Bool HAL_GetPCMCompressorClosedLoopControl(HAL_PCMHandle handle,
                                               int32_t* status);

/**
 * Gets the compressor pressure switch state.
 *
 * @param  handle the module handle
 * @return true if the pressure switch is triggered, otherwise false
 */
HAL_Bool HAL_GetPCMCompressorPressureSwitch(HAL_PCMHandle handle,
                                            int32_t* status);

/**
 * Gets the compressor current.
 *
 * @param  handle the module handle
 * @return the compressor current in amps
 */
double HAL_GetPCMCompressorCurrent(HAL_PCMHandle handle, int32_t* status);

/**
 * Gets if the compressor is faulted because of too high of current.
 *
 * @param  handle the module handle
 * @return true if falted, otherwise false
 */
HAL_Bool HAL_GetPCMCompressorCurrentTooHighFault(HAL_PCMHandle handle,
                                                 int32_t* status);

/**
 * Gets if a sticky fauly is triggered because of too high of current.
 *
 * @param  handle the module handle
 * @return true if falted, otherwise false
 */
HAL_Bool HAL_GetPCMCompressorCurrentTooHighStickyFault(HAL_PCMHandle handle,
                                                       int32_t* status);

/**
 * Gets if a sticky fauly is triggered because of a short.
 *
 * @param  handle the module handle
 * @return true if falted, otherwise false
 */
HAL_Bool HAL_GetPCMCompressorShortedStickyFault(HAL_PCMHandle handle,
                                                int32_t* status);

/**
 * Gets if the compressor is faulted because of a short.
 *
 * @param  handle the module handle
 * @return true if shorted, otherwise false
 */
HAL_Bool HAL_GetPCMCompressorShortedFault(HAL_PCMHandle handle,
                                          int32_t* status);

/**
 * Gets if a sticky fault is triggered of the compressor not connected.
 *
 * @param  handle the module handle
 * @return true if falted, otherwise false
 */
HAL_Bool HAL_GetPCMCompressorNotConnectedStickyFault(HAL_PCMHandle handle,
                                                     int32_t* status);

/**
 * Gets if the compressor is not connected.
 *
 * @param handle the module handle
 * @return                 true if not connected, otherwise false
 */
HAL_Bool HAL_GetPCMCompressorNotConnectedFault(HAL_PCMHandle handle,
                                               int32_t* status);

/**
 * Gets the channels blacklisted from being enabled on a module.
 *
 * @param handle the module handle
 * @retur        bitmask of the blacklisted channels, 1 for true 0 for false
 */
int32_t HAL_GetPCMSolenoidBlackList(HAL_PCMHandle handle, int32_t* status);

/**
 * Gets if a specific module has an over or under voltage sticky fault.
 *
 * @param handle the module handle
 * @return       true if a stick fault is set, otherwise false
 */
HAL_Bool HAL_GetPCMSolenoidVoltageStickyFault(HAL_PCMHandle handle,
                                              int32_t* status);

/**
 * Gets if a specific module has an over or under voltage fault.
 *
 * @param handle the module handle
 * @return       true if faulted, otherwise false
 */
HAL_Bool HAL_GetPCMSolenoidVoltageFault(HAL_PCMHandle handle, int32_t* status);

/**
 * Clears all faults on a module.
 *
 * @param handle the module handle
 */
void HAL_ClearAllPCMStickyFaults(HAL_PCMHandle handle, int32_t* status);

/**
 * Gets if a PCM module is enabled.
 *
 * @param handel the module handle
 * @return       true is enabled, otherwise false
 */
HAL_Bool HAL_GetPCMModuleEnabled(HAL_PCMHandle handle, int32_t* status);

#ifdef __cplusplus
}  // extern "C"
#endif
/** @} */

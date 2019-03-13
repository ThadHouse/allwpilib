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
 * This will be ref counted
 */
HAL_PCMHandle HAL_InitializePCM(int32_t module, int32_t* status);

/**
 * The ret val is if was the final reference or not
 */
HAL_Bool HAL_CleanPCM(HAL_PCMHandle handle);

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

void HAL_SetAllPCMSolenoids(HAL_PCMHandle handle, int32_t state, int32_t* status);

void HAL_SetPCMSolenoid(HAL_PCMHandle handle, int32_t port, HAL_Bool value, int32_t* status);

int32_t HAL_GetAllPCMSolenoids(HAL_PCMHandle handle, int32_t* status);

HAL_Bool HAL_GetPCMSolenoid(HAL_PCMHandle handle, int32_t port, int32_t* status);

void HAL_SetPCMClosedLoopControl(HAL_PCMHandle handle, HAL_Bool closedLoop, int32_t* status);

void HAL_ClearPCMStickyFaults(HAL_PCMHandle handle, int32_t* status);

HAL_Bool HAL_GetPCMAtPressure(HAL_PCMHandle handle, int32_t* status);

HAL_Bool HAL_GetPCMCompressorEnabled(HAL_PCMHandle handle, int32_t* status);

HAL_Bool HAL_GetPCMCompressorClosedLoopControl(HAL_PCMHandle handle, int32_t* status);

double HAL_GetPCMCompressorCurrent(HAL_PCMHandle handle, int32_t* status);

double HAL_GetPCMSolenoidVoltage(HAL_PCMHandle handle, int32_t* status);

double HAL_GetPCMInputVoltage(HAL_PCMHandle handle, int32_t* status);

HAL_Bool HAL_GetPCMHardwareFault(HAL_PCMHandle handle, int32_t* status);

HAL_Bool HAL_GetPCMCompressorCurrentTooHighFault(HAL_PCMHandle handle, int32_t* status);

HAL_Bool HAL_GetPCMSolenoidFault(HAL_PCMHandle handle, int32_t* status);

HAL_Bool HAL_GetPCMCompressorShortedFault(HAL_PCMHandle handle, int32_t* status);

HAL_Bool HAL_GetPCMCompressorNotConnectedFault(HAL_PCMHandle handle, int32_t* status);

HAL_Bool HAL_GetPCMCompressorCurrentTooHighStickyFault(HAL_PCMHandle handle, int32_t* status);

HAL_Bool HAL_GetPCMSolenoidStickyFault(HAL_PCMHandle handle, int32_t* status);

HAL_Bool HAL_GetPCMCompressorShortedStickyFault(HAL_PCMHandle handle, int32_t* status);

HAL_Bool HAL_GetPCMCompressorNotConnectedStickyFault(HAL_PCMHandle handle, int32_t* status);

int32_t HAL_GetPCMNumberOfFailedControlFrames(HAL_PCMHandle handle, int32_t* status);

HAL_Bool HAL_GetPCMIsSolenoidBlacklisted(HAL_PCMHandle handle, int32_t port, int32_t* status);

HAL_Bool HAL_GetPCMEnabled(HAL_PCMHandle handle, int32_t* status);

void HAL_FirePCMOneShotSolenoid(HAL_PCMHandle handle, int32_t port, int32_t* status);

void HAL_SetPCMOneShotDuration(HAL_PCMHandle handle, int32_t port, int32_t width, int32_t* status);


#ifdef __cplusplus
}  // extern "C"
#endif
/** @} */

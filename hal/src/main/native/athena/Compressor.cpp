/*----------------------------------------------------------------------------*/
/* Copyright (c) 2016-2018 FIRST. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "hal/Compressor.h"

#include "HALInitializer.h"
#include "PortsInternal.h"
#include "hal/Errors.h"
#include "hal/PCM.h"
#include "hal/handles/HandlesInternal.h"

using namespace hal;

namespace hal {
namespace init {
void InitializeCompressor() {}
}  // namespace init
}  // namespace hal

extern "C" {

HAL_CompressorHandle HAL_InitializeCompressor(int32_t module, int32_t* status) {
  hal::init::CheckInit();
  return static_cast<HAL_CompressorHandle>(HAL_InitializeCompressor(module, status));
}

HAL_CompressorHandle HAL_InitializeCompressorFromPCM(HAL_PCMHandle handle, int32_t* status) {
  HAL_IncremementPCMRefCountAndGetModuleNumber(handle, status);
  if (*status != 0) {
    return HAL_kInvalidHandle;
  } else {
    return static_cast<HAL_CompressorHandle>(handle);
  }
}

void HAL_CleanCompressor(HAL_CompressorHandle handle) {
  HAL_CleanPCM(handle);
}

HAL_Bool HAL_CheckCompressorModule(int32_t module) {
  return module < kNumPCMModules && module >= 0;
}

HAL_Bool HAL_GetCompressorEnabled(HAL_CompressorHandle compressorHandle,
                           int32_t* status) {
  return HAL_GetPCMCompressorEnabled(compressorHandle, status);
}

void HAL_SetCompressorClosedLoopControl(HAL_CompressorHandle compressorHandle,
                                        HAL_Bool value, int32_t* status) {
  HAL_SetPCMCompressorClosedLoopControl(compressorHandle, value, status);
}

HAL_Bool HAL_GetCompressorClosedLoopControl(
    HAL_CompressorHandle compressorHandle, int32_t* status) {
  return HAL_GetPCMCompressorClosedLoopControl(compressorHandle, status);
}

HAL_Bool HAL_GetCompressorAtPressure(HAL_CompressorHandle compressorHandle,
                                         int32_t* status) {
  return HAL_GetPCMCompressorAtPressure(compressorHandle, status);
}

double HAL_GetCompressorCurrent(HAL_CompressorHandle compressorHandle,
                                int32_t* status) {
  return HAL_GetPCMCompressorCurrent(compressorHandle, status);
}
HAL_Bool HAL_GetCompressorCurrentTooHighFault(
    HAL_CompressorHandle compressorHandle, int32_t* status) {
  return HAL_GetPCMCompressorCurrentTooHighFault(compressorHandle, status);
}

HAL_Bool HAL_GetCompressorCurrentTooHighStickyFault(
    HAL_CompressorHandle compressorHandle, int32_t* status) {
  return HAL_GetPCMCompressorCurrentTooHighStickyFault(compressorHandle, status);
}

HAL_Bool HAL_GetCompressorShortedStickyFault(
    HAL_CompressorHandle compressorHandle, int32_t* status) {
  return HAL_GetPCMCompressorShortedStickyFault(compressorHandle, status);
}
HAL_Bool HAL_GetCompressorShortedFault(HAL_CompressorHandle compressorHandle,
                                       int32_t* status) {
  return HAL_GetPCMCompressorShortedFault(compressorHandle, status);
}
HAL_Bool HAL_GetCompressorNotConnectedStickyFault(
    HAL_CompressorHandle compressorHandle, int32_t* status) {
  return HAL_GetPCMCompressorNotConnectedStickyFault(compressorHandle, status);
}
HAL_Bool HAL_GetCompressorNotConnectedFault(
    HAL_CompressorHandle compressorHandle, int32_t* status) {
  return HAL_GetPCMCompressorNotConnectedFault(compressorHandle, status);
}

}  // extern "C"

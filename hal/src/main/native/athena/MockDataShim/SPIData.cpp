/*----------------------------------------------------------------------------*/
/* Copyright (c) 2017 FIRST. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "MockData/SPIData.h"

extern "C" {

void HALSIM_ResetSPIData(int32_t index) {}

int32_t HALSIM_RegisterSPIInitializedCallback(int32_t index,
                                              HAL_NotifyCallback callback,
                                              void* param,
                                              HAL_Bool initialNotify) {
  return 0;
}
void HALSIM_CancelSPIInitializedCallback(int32_t index, int32_t uid) {}
HAL_Bool HALSIM_GetSPIInitialized(int32_t index) { return false; }
void HALSIM_SetSPIInitialized(int32_t index, HAL_Bool initialized) {}

int32_t HALSIM_RegisterSPIReadCallback(int32_t index,
                                       HAL_BufferCallback callback,
                                       void* param) {
  return 0;
}
void HALSIM_CancelSPIReadCallback(int32_t index, int32_t uid) {}

int32_t HALSIM_RegisterSPIWriteCallback(int32_t index,
                                        HAL_ConstBufferCallback callback,
                                        void* param) {
  return 0;
}
void HALSIM_CancelSPIWriteCallback(int32_t index, int32_t uid) {}

int32_t HALSIM_RegisterSPIResetAccumulatorCallback(int32_t index,
                                                   HAL_NotifyCallback callback,
                                                   void* param,
                                                   HAL_Bool initialNotify) {
  return 0;
}
void HALSIM_CancelSPIResetAccumulatorCallback(int32_t index, int32_t uid) {}

int32_t HALSIM_RegisterSPISetAccumulatorCallback(int32_t index,
                                                 HAL_BufferCallback callback,
                                                 void* param) {
  return 0;
}
void HALSIM_CancelSPISetAccumulatorCallback(int32_t index, int32_t uid) {}
void HALSIM_SetSPISetAccumulatorValue(int32_t index, int64_t value) {}
int64_t HALSIM_GetSPIGetAccumulatorValue(int32_t index) { return 0; }

}  // extern "C"

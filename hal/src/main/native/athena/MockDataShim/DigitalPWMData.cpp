/*----------------------------------------------------------------------------*/
/* Copyright (c) 2017 FIRST. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "MockData/DigitalPWMData.h"

extern "C" {

void HALSIM_ResetDigitalPWMData(int32_t index) {}
int32_t HALSIM_RegisterDigitalPWMInitializedCallback(
    int32_t index, HAL_NotifyCallback callback, void* param,
    HAL_Bool initialNotify) {
  return 0;
}
void HALSIM_CancelDigitalPWMInitializedCallback(int32_t index, int32_t uid) {}
HAL_Bool HALSIM_GetDigitalPWMInitialized(int32_t index) { return false; }
void HALSIM_SetDigitalPWMInitialized(int32_t index, HAL_Bool initialized) {}

int32_t HALSIM_RegisterDigitalPWMDutyCycleCallback(int32_t index,
                                                   HAL_NotifyCallback callback,
                                                   void* param,
                                                   HAL_Bool initialNotify) {
  return 0;
}
void HALSIM_CancelDigitalPWMDutyCycleCallback(int32_t index, int32_t uid) {}
double HALSIM_GetDigitalPWMDutyCycle(int32_t index) { return 0; }
void HALSIM_SetDigitalPWMDutyCycle(int32_t index, double dutyCycle) {}

int32_t HALSIM_RegisterDigitalPWMPinCallback(int32_t index,
                                             HAL_NotifyCallback callback,
                                             void* param,
                                             HAL_Bool initialNotify) {
  return 0;
}
void HALSIM_CancelDigitalPWMPinCallback(int32_t index, int32_t uid) {}
int32_t HALSIM_GetDigitalPWMPin(int32_t index) { return 0; }
void HALSIM_SetDigitalPWMPin(int32_t index, int32_t pin) {}

void HALSIM_RegisterDigitalPWMAllCallbacks(int32_t index,
                                           HAL_NotifyCallback callback,
                                           void* param,
                                           HAL_Bool initialNotify) {}

}  // extern "C"

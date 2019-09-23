/*----------------------------------------------------------------------------*/
/* Copyright (c) 2019 FIRST. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#pragma once

#include <stdint.h>

#include <hal/Types.h>
#include <hal/Value.h>

#ifdef __cplusplus
#include <functional>

namespace halsimgui {

class SimDeviceGui {
 public:
  static void Initialize();
  static void Execute();

  // custom support
  static void Hide(const char* name);
  static void Add(std::function<void()> execute);
  static bool DisplayValue(const char* name, bool readonly, HAL_Value* value,
                           const char** options = nullptr,
                           int32_t numOptions = 0);
};

}  // namespace halsimgui

#endif  // __cplusplus

extern "C" {

void HALSIMGUI_DeviceTreeHide(const char* name);
void HALSIMGUI_DeviceTreeAdd(void* param, void (*execute)(void*));
HAL_Bool HALSIMGUI_DeviceTreeDisplayValue(const char* name, HAL_Bool readonly,
                                          struct HAL_Value* value,
                                          const char** options,
                                          int32_t numOptions);

}  // extern "C"

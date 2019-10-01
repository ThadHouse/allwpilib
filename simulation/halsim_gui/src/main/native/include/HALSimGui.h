/*----------------------------------------------------------------------------*/
/* Copyright (c) 2019 FIRST. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#pragma once

#ifdef __cplusplus

#include <functional>

namespace halsimgui {

class HALSimGui {
 public:
  static void Add(std::function<void()> initialize,
                  std::function<void()> execute);
  static bool Initialize();
  static void Main(void*);
  static void Exit(void*);
};

}  // namespace halsimgui

#endif  // __cplusplus

extern "C" {

void HALSIMGUI_Add(void* param, void (*initialize)(void*),
                   void (*execute)(void*));

}  // extern "C"

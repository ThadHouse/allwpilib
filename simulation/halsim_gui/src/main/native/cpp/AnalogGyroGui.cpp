/*----------------------------------------------------------------------------*/
/* Copyright (c) 2019 FIRST. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "AnalogGyroGui.h"

#include <cstdio>

#include <hal/Ports.h>
#include <hal/Value.h>
#include <mockdata/AnalogGyroData.h>

#include <imgui.h>

#include "SimDeviceGui.h"

using namespace halsimgui;

static void DisplayAnalogGyros() {
  static int numAccum = HAL_GetNumAccumulators();
  for (int i = 0; i < numAccum; ++i) {
    if (!HALSIM_GetAnalogGyroInitialized(i)) continue;
    char name[32];
    std::snprintf(name, sizeof(name), "AnalogGyro[%d]", i);
    if (ImGui::TreeNodeEx(name, ImGuiTreeNodeFlags_DefaultOpen)) {
      HAL_Value value;

      // angle
      value = HAL_MakeDouble(HALSIM_GetAnalogGyroAngle(i));
      if (SimDeviceGui::DisplayValue("Angle", false, &value))
        HALSIM_SetAnalogGyroAngle(i, value.data.v_double);

      // rate
      value = HAL_MakeDouble(HALSIM_GetAnalogGyroRate(i));
      if (SimDeviceGui::DisplayValue("Rate", false, &value))
        HALSIM_SetAnalogGyroRate(i, value.data.v_double);

      ImGui::TreePop();
    }
  }
}

void AnalogGyroGui::Initialize() { SimDeviceGui::Add(DisplayAnalogGyros); }

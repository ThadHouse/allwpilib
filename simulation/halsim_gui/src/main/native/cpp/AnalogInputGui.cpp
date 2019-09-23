/*----------------------------------------------------------------------------*/
/* Copyright (c) 2019 FIRST. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "AnalogInputGui.h"

#include <cstdio>

#include <hal/Ports.h>
#include <mockdata/AnalogGyroData.h>
#include <mockdata/AnalogInData.h>

#include <imgui.h>

#include "HALSimGui.h"

using namespace halsimgui;

static void DisplayAnalogInputs() {
  ImGui::Text("Use Ctrl+Click to edit value");
  bool hasInputs = false;
  static int numAnalog = HAL_GetNumAnalogInputs();
  static int numAccum = HAL_GetNumAccumulators();
  for (int i = 0; i < numAnalog; ++i) {
    if (HALSIM_GetAnalogInInitialized(i)) {
      hasInputs = true;
      char name[32];
      std::snprintf(name, sizeof(name), "analog %d", i);
      if (i < numAccum && HALSIM_GetAnalogGyroInitialized(i)) {
        ImGui::LabelText(name, "AnalogGyro[%d]", i);
      } else {
        float val = HALSIM_GetAnalogInVoltage(i);
        if (ImGui::SliderFloat(name, &val, 0.0, 5.0))
          HALSIM_SetAnalogInVoltage(i, val);
      }
    }
  }
  if (!hasInputs) ImGui::Text("No analog inputs");
}

void AnalogInputGui::Initialize() {
  HALSimGui::AddWindow("Analog Inputs", DisplayAnalogInputs,
                       ImGuiWindowFlags_AlwaysAutoResize);
}

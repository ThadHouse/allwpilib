/*----------------------------------------------------------------------------*/
/* Copyright (c) 2019 FIRST. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "AnalogOutGui.h"

#include <cstdio>
#include <cstring>

#include <hal/Ports.h>
#include <mockdata/AnalogOutData.h>

#include <imgui.h>

#include "HALSimGui.h"

using namespace halsimgui;

static void DisplayAnalogOutputs() {
  bool hasOutputs = false;
  bool first = true;
  static const int numAnalog = HAL_GetNumAnalogOutputs();
  for (int i = 0; i < numAnalog; ++i) {
    if (HALSIM_GetAnalogOutInitialized(i)) {
      hasOutputs = true;

      if (!first)
        ImGui::Separator();
      else
        first = false;

      char name[32];
      std::snprintf(name, sizeof(name), "Analog Out %d", i);
      float val = HALSIM_GetAnalogOutVoltage(i);
      ImGui::Value(name, val, "%0.3f");
    }
  }
  if (!hasOutputs) ImGui::Text("No analog outputs");
}

void AnalogOutGui::Initialize() {
  HALSimGui::AddWindow("Analog Outputs", DisplayAnalogOutputs,
                       ImGuiWindowFlags_AlwaysAutoResize);
}

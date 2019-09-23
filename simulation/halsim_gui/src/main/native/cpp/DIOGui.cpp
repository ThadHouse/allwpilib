/*----------------------------------------------------------------------------*/
/* Copyright (c) 2019 FIRST. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "DIOGui.h"

#include <cstdio>

#include <hal/Ports.h>
#include <mockdata/DigitalPWMData.h>
#include <mockdata/DIOData.h>
#include <wpi/SmallVector.h>

#include <imgui.h>

#include "HALSimGui.h"

using namespace halsimgui;

static void DisplayDIO() {
  bool hasAny = false;
  static int numDIO = HAL_GetNumDigitalChannels();
  static int numPWM = HAL_GetNumDigitalPWMOutputs();

  wpi::SmallVector<int, 32> pwmMap;
  pwmMap.resize(numDIO);
  for (int i = 0; i < numPWM; ++i) {
    if (HALSIM_GetDigitalPWMInitialized(i)) {
      int channel = HALSIM_GetDigitalPWMPin(i);
      if (channel >= 0 && channel < numDIO) pwmMap[channel] = i + 1;
    }
  }

  ImGui::PushItemWidth(100);
  for (int i = 0; i < numDIO; ++i) {
    if (HALSIM_GetDIOInitialized(i)) {
      hasAny = true;
      char name[32];
      std::snprintf(name, sizeof(name), "DIO %d", i);
      if (pwmMap[i] > 0) {
        ImGui::LabelText(name, "[PWM] %0.3f",
                         HALSIM_GetDigitalPWMDutyCycle(pwmMap[i] - 1));
      } else if (!HALSIM_GetDIOIsInput(i)) {
        ImGui::LabelText(name, "%s",
                         HALSIM_GetDIOValue(i) ? "1 (high)" : "0 (low)");
      } else {
        static const char* options[] = {"0 (low)", "1 (high)"};
        int val = HALSIM_GetDIOValue(i) ? 1 : 0;
        if (ImGui::Combo(name, &val, options, 2)) HALSIM_SetDIOValue(i, val);
      }
    }
  }
  ImGui::PopItemWidth();
  if (!hasAny) ImGui::Text("No DIO");
}

void DIOGui::Initialize() {
  HALSimGui::AddWindow("DIO", DisplayDIO, ImGuiWindowFlags_AlwaysAutoResize);
}

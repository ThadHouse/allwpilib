/*----------------------------------------------------------------------------*/
/* Copyright (c) 2019 FIRST. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "SolenoidGui.h"

#include <cstdio>
#include <cstring>

#include <hal/Ports.h>
#include <mockdata/PCMData.h>
#include <wpi/SmallVector.h>

#include <imgui.h>

using namespace halsimgui;

void SolenoidGui::Initialize() {}

void SolenoidGui::Execute() {
  if (ImGui::Begin("Solenoids", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    bool hasOutputs = false;
    static const int numPCM = HAL_GetNumPCMModules();
    static const int numChannels = HAL_GetNumSolenoidChannels();
    for (int i = 0; i < numPCM; ++i) {
      bool anyInit = false;
      wpi::SmallVector<int, 16> channels;
      channels.resize(numChannels);
      for (int j = 0; j < numChannels; ++j) {
        if (HALSIM_GetPCMSolenoidInitialized(i, j)) {
          anyInit = true;
          channels[j] = HALSIM_GetPCMSolenoidOutput(i, j);
        } else {
          channels[j] = -1;
        }
      }

      if (!anyInit) continue;
      hasOutputs = true;

      ImGui::Text("PCM %d", i);
      ImGui::SameLine();

      // show channels as LED indicators
      ImDrawList* drawList = ImGui::GetWindowDrawList();
      static constexpr float sz = 8.0;
      const ImVec2 p = ImGui::GetCursorScreenPos();
      static constexpr ImU32 color = IM_COL32(255, 255, 102, 255);
      static constexpr ImU32 gray = IM_COL32(128, 128, 128, 255);
      float x = p.x + 4.0, y = p.y + 4.0;
      static constexpr float spacing = 6.0;
      for (int j = 0; j < numChannels; ++j) {
        if (channels[j] == -1)
          drawList->AddRect(ImVec2(x, y), ImVec2(x + sz, y + sz), gray, 0.0f,
                            0, 1.0);
        else if (channels[j])
          drawList->AddRectFilled(ImVec2(x, y), ImVec2(x + sz, y + sz), color);
        else
          drawList->AddRect(ImVec2(x, y), ImVec2(x + sz, y + sz), color, 0.0f,
                            0, 1.0);
        x += sz + spacing;
      }
      ImGui::Dummy(ImVec2((sz + spacing) * 8, sz + spacing));
    }
    if (!hasOutputs) ImGui::Text("No solenoids");
  }
  ImGui::End();
}

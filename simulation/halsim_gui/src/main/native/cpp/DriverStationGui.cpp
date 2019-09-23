/*----------------------------------------------------------------------------*/
/* Copyright (c) 2019 FIRST. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "DriverStationGui.h"

#include <cstring>
#include <string>

#include <mockdata/DriverStationData.h>
#include <wpi/Format.h>
#include <wpi/SmallString.h>
#include <wpi/StringRef.h>
#include <wpi/raw_ostream.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <GLFW/glfw3.h>

using namespace halsimgui;

namespace {

struct SystemJoystick {
  bool present = false;
  int axisCount = 0;
  const float* axes = nullptr;
  int buttonCount = 0;
  const unsigned char* buttons = nullptr;
  int hatCount = 0;
  const unsigned char* hats = nullptr;
  const char* name = nullptr;
  bool isGamepad = false;
  GLFWgamepadstate gamepadState;

  bool anyButtonPressed = false;

  void Update(int i);
};

struct RobotJoystick {
  HAL_JoystickDescriptor desc;
  HAL_JoystickAxes axes;
  HAL_JoystickButtons buttons;
  HAL_JoystickPOVs povs;

  void Update(const SystemJoystick* sys);
  void SetHAL(int i);
};

}  // namespace

// system joysticks
static SystemJoystick gSystemJoysticks[GLFW_JOYSTICK_LAST + 1];

// robot joysticks
static RobotJoystick gRobotJoysticks[HAL_kMaxJoysticks];
static int gJoystickMap[HAL_kMaxJoysticks] = {-1, -1, -1, -1, -1, -1};
static std::string gJoystickGuids[HAL_kMaxJoysticks];

// read/write joystick mapping to ini file
static void* JoysticksReadOpen(ImGuiContext* ctx, ImGuiSettingsHandler* handler,
                               const char* name) {
  return &gJoystickGuids;
}

static void JoysticksReadLine(ImGuiContext* ctx, ImGuiSettingsHandler* handler,
                              void* entry, const char* lineStr) {
  // format: Joy##=guid
  wpi::StringRef line{lineStr};
  auto [name, guid] = line.split('=');
  name = name.trim();
  guid = guid.trim();
  if (!name.startswith("Joy")) return;
  int num;
  if (name.substr(3).getAsInteger(10, num)) return;
  if (num < 0 || num >= HAL_kMaxJoysticks) return;
  gJoystickGuids[num] = guid;
}

static void JoysticksWriteAll(ImGuiContext* ctx, ImGuiSettingsHandler* handler,
                              ImGuiTextBuffer* out_buf) {
  out_buf->append("[Joysticks][Joysticks]\n");
  for (int i = 0; i < HAL_kMaxJoysticks; ++i) {
    if (gJoystickMap[i] == -1) continue;
    const char* guid = glfwGetJoystickGUID(gJoystickMap[i]);
    if (!guid) continue;
    out_buf->appendf("Joy%d=%s\n", i, guid);
  }
}

void DriverStationGui::Initialize() {
  // hook ini handler to save joystick settings
  ImGuiSettingsHandler iniHandler;
  iniHandler.TypeName = "Joysticks";
  iniHandler.TypeHash = ImHashStr(iniHandler.TypeName);
  iniHandler.ReadOpenFn = JoysticksReadOpen;
  iniHandler.ReadLineFn = JoysticksReadLine;
  iniHandler.WriteAllFn = JoysticksWriteAll;
  ImGui::GetCurrentContext()->SettingsHandlers.push_back(iniHandler);
}

void SystemJoystick::Update(int i) {
  bool wasPresent = present;
  present = glfwJoystickPresent(i);

  if (!present) return;
  axes = glfwGetJoystickAxes(i, &axisCount);
  buttons = glfwGetJoystickButtons(i, &buttonCount);
  hats = glfwGetJoystickHats(i, &hatCount);
  isGamepad = glfwGetGamepadState(i, &gamepadState);

  anyButtonPressed = false;
  for (int j = 0; j < buttonCount; ++j) {
    if (buttons[j]) {
      anyButtonPressed = true;
      break;
    }
  }

  if (!present || wasPresent) return;
  name = glfwGetJoystickName(i);
  if (isGamepad) name = glfwGetGamepadName(i);

  // try to find matching GUID
  if (const char* guid = glfwGetJoystickGUID(i)) {
    for (int j = 0; j < HAL_kMaxJoysticks; ++j) {
      if (guid == gJoystickGuids[j]) {
        gJoystickMap[j] = i;
        gJoystickGuids[j].clear();
        break;
      }
    }
  }
}

static int HatToAngle(unsigned char hat) {
  switch (hat) {
    case GLFW_HAT_UP:
      return 0;
    case GLFW_HAT_RIGHT:
      return 90;
    case GLFW_HAT_DOWN:
      return 180;
    case GLFW_HAT_LEFT:
      return 270;
    case GLFW_HAT_RIGHT_UP:
      return 45;
    case GLFW_HAT_RIGHT_DOWN:
      return 135;
    case GLFW_HAT_LEFT_UP:
      return 315;
    case GLFW_HAT_LEFT_DOWN:
      return 225;
    default:
      return -1;
  }
}

void RobotJoystick::Update(const SystemJoystick* sys) {
  std::memset(&desc, 0, sizeof(desc));
  desc.type = -1;
  std::memset(&axes, 0, sizeof(axes));
  std::memset(&buttons, 0, sizeof(buttons));
  std::memset(&povs, 0, sizeof(povs));

  if (!sys) return;

  // use gamepad mappings if present
  const float* sysAxes;
  const unsigned char* sysButtons;
  if (sys->isGamepad) {
    sysAxes = sys->gamepadState.axes;
    sysButtons = sys->gamepadState.buttons;
  } else {
    sysAxes = sys->axes;
    sysButtons = sys->buttons;
  }

  // copy into HAL structures
  desc.isXbox = sys->isGamepad ? 1 : 0;
  desc.type = sys->isGamepad ? 21 : 20;
  std::strncpy(desc.name, sys->name, 256);
  desc.axisCount = (std::min)(sys->axisCount, HAL_kMaxJoystickAxes);
  // desc.axisTypes ???
  desc.buttonCount = (std::min)(sys->buttonCount, 32);
  desc.povCount = (std::min)(sys->hatCount, HAL_kMaxJoystickPOVs);

  axes.count = desc.axisCount;
  std::memcpy(axes.axes, sysAxes, axes.count * sizeof(&axes.axes[0]));

  buttons.count = desc.buttonCount;
  for (int j = 0; j < buttons.count; ++j)
    buttons.buttons |= (sysButtons[j] ? 1u : 0u) << j;

  povs.count = desc.povCount;
  for (int j = 0; j < povs.count; ++j) povs.povs[j] = HatToAngle(sys->hats[j]);
}

void RobotJoystick::SetHAL(int i) {
  // set at HAL level
  HALSIM_SetJoystickDescriptor(i, &desc);
  HALSIM_SetJoystickAxes(i, &axes);
  HALSIM_SetJoystickButtons(i, &buttons);
  HALSIM_SetJoystickPOVs(i, &povs);
}

static void UpdateJoysticks() {
  // update system joysticks
  for (int i = 0; i <= GLFW_JOYSTICK_LAST; ++i) gSystemJoysticks[i].Update(i);

  // update robot joysticks
  for (int i = 0; i < HAL_kMaxJoysticks; ++i) {
    int rawIndex = gJoystickMap[i];
    if (rawIndex != -1 && gSystemJoysticks[rawIndex].present)
      gRobotJoysticks[i].Update(&gSystemJoysticks[rawIndex]);
    else
      gRobotJoysticks[i].Update(nullptr);
  }
}

void DriverStationGui::Execute() {
  double curTime = glfwGetTime();

  UpdateJoysticks();

  bool isEnabled = HALSIM_GetDriverStationEnabled();
  bool isAuto = HALSIM_GetDriverStationAutonomous();
  bool isTest = HALSIM_GetDriverStationTest();

  // Robot state
  {
    ImGui::Begin("Robot State", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    if (ImGui::Selectable("Disabled", !isEnabled))
      HALSIM_SetDriverStationEnabled(false);
    if (ImGui::Selectable("Autonomous", isEnabled && isAuto && !isTest)) {
      HALSIM_SetDriverStationAutonomous(true);
      HALSIM_SetDriverStationTest(false);
      HALSIM_SetDriverStationEnabled(true);
    }
    if (ImGui::Selectable("Teleoperated", isEnabled && !isAuto && !isTest)) {
      HALSIM_SetDriverStationAutonomous(false);
      HALSIM_SetDriverStationTest(false);
      HALSIM_SetDriverStationEnabled(true);
    }
    if (ImGui::Selectable("Test", isEnabled && isTest)) {
      HALSIM_SetDriverStationAutonomous(false);
      HALSIM_SetDriverStationTest(true);
      HALSIM_SetDriverStationEnabled(true);
    }
    ImGui::End();
  }

  // FMS
  if (ImGui::Begin("FMS", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    // FMS Attached
    bool fmsAttached = HALSIM_GetDriverStationFmsAttached();
    if (ImGui::Checkbox("FMS Attached", &fmsAttached))
      HALSIM_SetDriverStationFmsAttached(fmsAttached);

    // DS Attached
    bool dsAttached = HALSIM_GetDriverStationDsAttached();
    if (ImGui::Checkbox("DS Attached", &dsAttached))
      HALSIM_SetDriverStationDsAttached(dsAttached);

    // Alliance Station
    static const char* stations[] = {"Red 1",  "Red 2",  "Red 3",
                                     "Blue 1", "Blue 2", "Blue 3"};
    int allianceStationId = HALSIM_GetDriverStationAllianceStationId();
    ImGui::SetNextItemWidth(100);
    if (ImGui::Combo("Alliance Station", &allianceStationId, stations, 6))
      HALSIM_SetDriverStationAllianceStationId(
          static_cast<HAL_AllianceStationID>(allianceStationId));

    // Match Time
    static bool matchTimeEnabled = true;
    ImGui::Checkbox("Match Time Enabled", &matchTimeEnabled);

    static double startMatchTime = 0.0;
    double matchTime = HALSIM_GetDriverStationMatchTime();
    ImGui::SetNextItemWidth(100);
    if (ImGui::InputDouble("Match Time", &matchTime, 0, 0, "%.1f",
                           ImGuiInputTextFlags_EnterReturnsTrue)) {
      HALSIM_SetDriverStationMatchTime(matchTime);
      startMatchTime = curTime - matchTime;
    } else if (!isEnabled) {
      startMatchTime = curTime - matchTime;
    } else if (matchTimeEnabled) {
      HALSIM_SetDriverStationMatchTime(curTime - startMatchTime);
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
      HALSIM_SetDriverStationMatchTime(0.0);
      startMatchTime = curTime;
    }

    // Game Specific Message
    static HAL_MatchInfo matchInfo;
    ImGui::SetNextItemWidth(100);
    if (ImGui::InputText("Game Specific",
                         reinterpret_cast<char*>(matchInfo.gameSpecificMessage),
                         sizeof(matchInfo.gameSpecificMessage),
                         ImGuiInputTextFlags_EnterReturnsTrue)) {
      matchInfo.gameSpecificMessageSize =
          std::strlen(reinterpret_cast<char*>(matchInfo.gameSpecificMessage));
      HALSIM_SetMatchInfo(&matchInfo);
    }
  }
  ImGui::End();

  // System joysticks
  if (ImGui::Begin("System Joysticks", nullptr,
                   ImGuiWindowFlags_AlwaysAutoResize)) {
    // drag and drop sources are the low level joysticks
    for (int i = 0; i <= GLFW_JOYSTICK_LAST; ++i) {
      auto& joy = gSystemJoysticks[i];
      wpi::SmallString<128> label;
      wpi::raw_svector_ostream os(label);
      os << wpi::format("%d: %s", i, joy.name);

      // highlight if any buttons pressed
      if (joy.anyButtonPressed)
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
      ImGui::Selectable(label.c_str(), false,
                        joy.present ? ImGuiSelectableFlags_None
                                    : ImGuiSelectableFlags_Disabled);
      if (joy.anyButtonPressed) ImGui::PopStyleColor();
      if (ImGui::BeginDragDropSource()) {
        ImGui::SetDragDropPayload("Joystick", &i, sizeof(i));
        ImGui::Text("%d: %s", i, joy.name);
        ImGui::EndDragDropSource();
      }
    }
  }
  ImGui::End();

  // Robot joysticks
  if (ImGui::Begin("Joysticks", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    // imgui doesn't size columns properly with autoresize, so force it
    ImGui::Dummy(ImVec2(14.0 * 9 * HAL_kMaxJoysticks, 0));

    ImGui::Columns(HAL_kMaxJoysticks, "Joysticks", false);
    for (int i = 0; i < HAL_kMaxJoysticks; ++i) {
      char label[30];
      sprintf(label, "Joystick %d", i);
      int rawIndex = gJoystickMap[i];
      if (rawIndex != -1 && gSystemJoysticks[rawIndex].anyButtonPressed)
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
      ImGui::Selectable(label, false,
                        rawIndex != -1 ? ImGuiSelectableFlags_None
                                       : ImGuiSelectableFlags_Disabled);
      if (rawIndex != -1 && gSystemJoysticks[rawIndex].anyButtonPressed)
        ImGui::PopStyleColor();
      if (rawIndex != -1) {
        auto& joy = gSystemJoysticks[rawIndex];
        if (ImGui::BeginDragDropSource()) {
          ImGui::SetDragDropPayload("Joystick", &rawIndex, sizeof(rawIndex));
          ImGui::Text("%d: %s", rawIndex, joy.name);
          ImGui::EndDragDropSource();
        }
      }
      if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload =
                ImGui::AcceptDragDropPayload("Joystick")) {
          IM_ASSERT(payload->DataSize == sizeof(int));
          int payload_i = *static_cast<const int*>(payload->Data);
          // clear it from the other joysticks
          for (int j = 0; j < HAL_kMaxJoysticks; ++j) {
            if (gJoystickMap[j] == payload_i) gJoystickMap[j] = -1;
          }
          gJoystickMap[i] = payload_i;
          gJoystickGuids[i].clear();
        }
        ImGui::EndDragDropTarget();
      }
      ImGui::NextColumn();
    }
    ImGui::Separator();

    for (int i = 0; i < HAL_kMaxJoysticks; ++i) {
      int rawIndex = gJoystickMap[i];

      if (rawIndex != -1 && gSystemJoysticks[rawIndex].present) {
        auto& sys = gSystemJoysticks[rawIndex];
        auto& joy = gRobotJoysticks[i];

        // update GUI display
        ImGui::Text("%d: %s", rawIndex, sys.name);

        for (int j = 0; j < joy.axes.count; ++j)
          ImGui::Text("Axis[%d]: %.3f", j, joy.axes.axes[j]);

        for (int j = 0; j < joy.povs.count; ++j)
          ImGui::Text("POVs[%d]: %d", j, joy.povs.povs[j]);

        // show buttons as multiple lines of LED indicators, 8 per line
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        static constexpr float sz = 8.0;
        const ImVec2 p = ImGui::GetCursorScreenPos();
        static constexpr ImU32 color = IM_COL32(255, 255, 102, 255);
        float x = p.x + 4.0, y = p.y + 4.0;
        static constexpr float spacing = 6.0;
        int rows = 1;
        for (int j = 0; j < joy.buttons.count; ++j) {
          if (j >= (rows * 8)) {
            ++rows;
            x = p.x + 4.0;
            y += sz + spacing;
          }
          if (sys.buttons[j])
            drawList->AddRectFilled(ImVec2(x, y), ImVec2(x + sz, y + sz),
                                    color);
          else
            drawList->AddRect(ImVec2(x, y), ImVec2(x + sz, y + sz), color, 0.0f,
                              0, 1.0);
          x += sz + spacing;
        }
        if (joy.buttons.count > 0)
          ImGui::Dummy(ImVec2((sz + spacing) * 8, (sz + spacing) * rows));
      } else {
        ImGui::Text("Unassigned");
      }
      ImGui::NextColumn();
    }
    ImGui::Columns(1);
  }
  ImGui::End();

  // Update HAL
  for (int i = 0; i < HAL_kMaxJoysticks; ++i) gRobotJoysticks[i].SetHAL(i);

  // Send new data every 20 ms (may be slower depending on GUI refresh rate)
  static double lastNewDataTime = 0.0;
  if ((curTime - lastNewDataTime) > 0.02) {
    lastNewDataTime = curTime;
    HALSIM_NotifyDriverStationNewData();
  }
}

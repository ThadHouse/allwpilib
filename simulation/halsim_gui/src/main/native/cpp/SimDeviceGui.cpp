/*----------------------------------------------------------------------------*/
/* Copyright (c) 2019 FIRST. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "SimDeviceGui.h"

#include <stdint.h>

#include <vector>

#include <hal/SimDevice.h>
#include <mockdata/SimDeviceData.h>
#include <wpi/StringMap.h>

#include <imgui.h>

using namespace halsimgui;

static std::vector<std::function<void()>> gDeviceExecutors;
static wpi::StringMap<bool> gHiddenElements;

void SimDeviceGui::Hide(const char* name) { gHiddenElements[name] = true; }

void SimDeviceGui::Add(std::function<void()> execute) {
  if (execute) gDeviceExecutors.emplace_back(std::move(execute));
}

bool DisplayValueImpl(const char* name, bool readonly, HAL_Value* value,
                      const char** options, int32_t numOptions) {
  // read-only
  if (readonly) {
    switch (value->type) {
      case HAL_BOOLEAN:
        ImGui::LabelText(name, "%s", value->data.v_boolean ? "true" : "false");
        break;
      case HAL_DOUBLE:
        ImGui::LabelText(name, "%.6f", value->data.v_double);
        break;
      case HAL_ENUM: {
        int current = value->data.v_enum;
        if (current < 0 || current >= numOptions)
          ImGui::LabelText(name, "%d (unknown)", current);
        else
          ImGui::LabelText(name, "%s", options[current]);
        break;
      }
      case HAL_INT:
        ImGui::LabelText(name, "%d", static_cast<int>(value->data.v_int));
        break;
      case HAL_LONG:
        ImGui::LabelText(name, "%lld",
                         static_cast<long long int>(
                             value->data.v_long));  // NOLINT(runtime/int)
        break;
      default:
        break;
    }
    return false;
  }

  // writable
  switch (value->type) {
    case HAL_BOOLEAN: {
      static const char* boolOptions[] = {"false", "true"};
      int val = value->data.v_boolean ? 1 : 0;
      if (ImGui::Combo(name, &val, boolOptions, 2)) {
        value->data.v_boolean = val;
        return true;
      }
      break;
    }
    case HAL_DOUBLE: {
      if (ImGui::InputDouble(name, &value->data.v_double, 0, 0, "%.6f",
                             ImGuiInputTextFlags_EnterReturnsTrue))
        return true;
      break;
    }
    case HAL_ENUM: {
      int current = value->data.v_enum;
      if (ImGui::Combo(name, &current, options, numOptions)) {
        value->data.v_enum = current;
        return true;
      }
      break;
    }
    case HAL_INT: {
      if (ImGui::InputScalar(name, ImGuiDataType_S32, &value->data.v_int,
                             nullptr, nullptr, nullptr,
                             ImGuiInputTextFlags_EnterReturnsTrue))
        return true;
      break;
    }
    case HAL_LONG: {
      if (ImGui::InputScalar(name, ImGuiDataType_S64, &value->data.v_long,
                             nullptr, nullptr, nullptr,
                             ImGuiInputTextFlags_EnterReturnsTrue))
        return true;
      break;
    }
    default:
      break;
  }
  return false;
}

bool SimDeviceGui::DisplayValue(const char* name, bool readonly,
                                HAL_Value* value, const char** options,
                                int32_t numOptions) {
  ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.5f);
  return DisplayValueImpl(name, readonly, value, options, numOptions);
}

static void SimDeviceDisplayValue(const char* name, void*,
                                  HAL_SimValueHandle handle, HAL_Bool readonly,
                                  const HAL_Value* value) {
  int32_t numOptions = 0;
  const char** options = nullptr;

  if (value->type == HAL_ENUM)
    options = HALSIM_GetSimValueEnumOptions(handle, &numOptions);

  HAL_Value valueCopy = *value;
  if (DisplayValueImpl(name, readonly, &valueCopy, options, numOptions))
    HAL_SetSimValue(handle, valueCopy);
}

static void SimDeviceDisplayDevice(const char* name, void*,
                                   HAL_SimDeviceHandle handle) {
  if (gHiddenElements.count(name) > 0) return;

  if (ImGui::TreeNodeEx(name, ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);
    HALSIM_EnumerateSimValues(handle, nullptr, SimDeviceDisplayValue);
    ImGui::PopItemWidth();
    ImGui::TreePop();
  }
}

void SimDeviceGui::Initialize() {}

void SimDeviceGui::Execute() {
  if (ImGui::Begin("Device Tree", nullptr)) {
    for (auto&& execute : gDeviceExecutors) {
      if (execute) execute();
    }
    HALSIM_EnumerateSimDevices("", nullptr, SimDeviceDisplayDevice);
  }
  ImGui::End();
}

extern "C" {

void HALSIMGUI_DeviceTreeAdd(void* param, void (*execute)(void*)) {
  if (execute) SimDeviceGui::Add([=] { execute(param); });
}

void HALSIMGUI_DeviceTreeHide(const char* name) { SimDeviceGui::Hide(name); }

HAL_Bool HALSIMGUI_DeviceTreeDisplayValue(const char* name, HAL_Bool readonly,
                                          struct HAL_Value* value,
                                          const char** options,
                                          int32_t numOptions) {
  return SimDeviceGui::DisplayValue(name, readonly, value, options, numOptions);
}

}  // extern "C"

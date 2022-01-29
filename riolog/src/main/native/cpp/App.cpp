// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

#include <memory>
#include <string_view>

#ifndef _WIN32
#include <arpa/inet.h>
#endif

#include <fmt/format.h>
#include <glass/MainMenuBar.h>
#include <glass/Context.h>
#include <glass/Storage.h>
#include <glass/Window.h>
#include <glass/WindowManager.h>
#include <glass/other/Log.h>
#include <imgui.h>
#include <wpi/Logger.h>
#include <wpi/fs.h>
#include <wpigui.h>
#include <unordered_map>
#include <mutex>
#include "wpi/SmallString.h"
#include "RioLog.h"

namespace gui = wpi::gui;

const char* GetWPILibVersion();

namespace riolog {
std::string_view GetResource_riolog_16_png();
std::string_view GetResource_riolog_32_png();
std::string_view GetResource_riolog_48_png();
std::string_view GetResource_riolog_64_png();
std::string_view GetResource_riolog_128_png();
std::string_view GetResource_riolog_256_png();
std::string_view GetResource_riolog_512_png();
}  // namespace riolog

#define GLFWAPI extern "C"
GLFWAPI void glfwGetWindowSize(GLFWwindow* window, int* width, int* height);
#define GLFW_DONT_CARE -1
GLFWAPI void glfwSetWindowSizeLimits(GLFWwindow* window, int minwidth,
                                     int minheight, int maxwidth,
                                     int maxheight);
GLFWAPI void glfwSetWindowSize(GLFWwindow* window, int width, int height);

struct TeamNumberRefHolder {
  explicit TeamNumberRefHolder(glass::Storage& storage)
      : teamNumber{storage.GetInt("TeamNumber", 0)} {}
  int& teamNumber;
};

static std::unique_ptr<TeamNumberRefHolder> teamNumberRef;
static wpi::Logger logger;
static glass::MainMenuBar gMainMenu;
static glass::LogData gLogData;
static glass::LogView gLogView{&gLogData};
static bool autoScroll = true;
static std::unique_ptr<rl::RioLog> rioLog;

static int minWidth = 400;

static void DisplayGui() {
  int& teamNumber = teamNumberRef->teamNumber;

  ImGui::GetStyle().WindowRounding = 0;

  // fill entire OS window with this window
  ImGui::SetNextWindowPos(ImVec2(0, 0));
  int width, height;
  glfwGetWindowSize(gui::GetSystemWindow(), &width, &height);

  ImGui::SetNextWindowSize(ImVec2(width, height));

  ImGui::Begin("Entries", nullptr,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar |
                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                   ImGuiWindowFlags_NoCollapse);

  ImGui::BeginMenuBar();
  gMainMenu.WorkspaceMenu();
  gui::EmitViewMenu();

  bool config = false;
  if (ImGui::BeginMenu("Control")) {
    if (ImGui::MenuItem("Clear")) {
      gLogData.Clear();
    }
    if (ImGui::MenuItem("Configure")) {
      config = true;
    }
    ImGui::EndMenu();
  }

  bool about = false;
  if (ImGui::BeginMenu("Info")) {
    if (ImGui::MenuItem("About")) {
      about = true;
    }
    ImGui::EndMenu();
  }
  ImGui::EndMenuBar();

  if (config) {
    ImGui::OpenPopup("Config");
  }
  if (ImGui::BeginPopupModal("Config")) {
    ImGui::Checkbox("Auto Scroll?", &autoScroll);
    if (ImGui::Button("Close")) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }

  if (about) {
    ImGui::OpenPopup("About");
  }
  if (ImGui::BeginPopupModal("About")) {
    ImGui::Text("riolog");
    ImGui::Separator();
    ImGui::Text("v%s", GetWPILibVersion());
    ImGui::Separator();
    ImGui::Text("Save location: %s", glass::GetStorageDir().c_str());
    if (ImGui::Button("Close")) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }

  if (ImGui::BeginPopupContextWindow()) {
    if (ImGui::MenuItem("Clear")) {
      gLogData.Clear();
    }
    ImGui::Checkbox("Auto Scroll?", &autoScroll);
    ImGui::EndPopup();
  }

  auto msgs = rioLog->GetMessages();
  for (auto&& msg : msgs) {
    if (std::holds_alternative<rl::Print>(msg.value)) {
      rl::Print& print = std::get<rl::Print>(msg.value);
      gLogData.Append(fmt::format("PRINT: {} {} {}\n", msg.timestamp, msg.sequenceNumber, print.line));
    }

  }

  glass::DisplayLog(&gLogData, autoScroll);

  ImGui::End();

  glfwSetWindowSizeLimits(gui::GetSystemWindow(), minWidth, 200, GLFW_DONT_CARE,
                          GLFW_DONT_CARE);
  if (width < minWidth) {
    width = minWidth;
    glfwSetWindowSize(gui::GetSystemWindow(), width, height);
  }
}

void Application(std::string_view saveDir) {
  gui::CreateContext();
  glass::CreateContext();

  // Add icons
  gui::AddIcon(riolog::GetResource_riolog_16_png());
  gui::AddIcon(riolog::GetResource_riolog_32_png());
  gui::AddIcon(riolog::GetResource_riolog_48_png());
  gui::AddIcon(riolog::GetResource_riolog_64_png());
  gui::AddIcon(riolog::GetResource_riolog_128_png());
  gui::AddIcon(riolog::GetResource_riolog_256_png());
  gui::AddIcon(riolog::GetResource_riolog_512_png());

  glass::SetStorageName("riolog");
  glass::SetStorageDir(saveDir.empty() ? gui::GetPlatformSaveFileDir()
                                       : saveDir);

  teamNumberRef =
      std::make_unique<TeamNumberRefHolder>(glass::GetStorageRoot());

  rioLog = std::make_unique<rl::RioLog>();
  auto addr = inet_addr("172.22.11.2");
  rioLog->SetDsIpAddress(addr);

  gui::AddLateExecute(DisplayGui);
  gui::Initialize("riolog", 600, 400);

  gui::Main();

  rioLog = nullptr;

  glass::DestroyContext();
  gui::DestroyContext();
}

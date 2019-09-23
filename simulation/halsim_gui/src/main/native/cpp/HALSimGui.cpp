/*----------------------------------------------------------------------------*/
/* Copyright (c) 2019 FIRST. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "HALSimGui.h"

#include <atomic>

#include <wpi/raw_ostream.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

using namespace halsimgui;

namespace {
struct WindowInfo {
  WindowInfo() = default;
  WindowInfo(const char* name_, std::function<void()> display_,
             ImGuiWindowFlags flags_)
      : name{name_}, display{std::move(display_)}, flags{flags_} {}

  std::string name;
  std::function<void()> display;
  ImGuiWindowFlags flags = 0;
  bool visible = true;
};
}  // namespace

static std::atomic_bool gExit{false};
static GLFWwindow* gWindow;
static std::vector<std::function<void()>> gInitializers;
static std::vector<std::function<void()>> gExecutors;
static std::vector<WindowInfo> gWindows;
static std::vector<std::function<void()>> gMenus;

static void glfw_error_callback(int error, const char* description) {
  wpi::errs() << "GLFW Error " << error << ": " << description << '\n';
}

void HALSimGui::Add(std::function<void()> initialize) {
  if (initialize) gInitializers.emplace_back(std::move(initialize));
}

void HALSimGui::AddExecute(std::function<void()> execute) {
  if (execute) gExecutors.emplace_back(std::move(execute));
}

void HALSimGui::AddWindow(const char* name, std::function<void()> display,
                          int flags) {
  if (display)
    gWindows.emplace_back(name, std::move(display),
                          static_cast<ImGuiWindowFlags>(flags));
}

void HALSimGui::AddMainMenu(std::function<void()> menu) {
  if (menu) gMenus.emplace_back(std::move(menu));
}

bool HALSimGui::Initialize() {
  // Setup window
  glfwSetErrorCallback(glfw_error_callback);
  glfwInitHint(GLFW_JOYSTICK_HAT_BUTTONS, GLFW_FALSE);
  if (!glfwInit()) return false;

    // Decide GL+GLSL versions
#if __APPLE__
  // GL 3.2 + GLSL 150
  const char* glsl_version = "#version 150";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);  // Required on Mac
#else
  // GL 3.0 + GLSL 130
  const char* glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+
  // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // 3.0+
#endif

  // Create window with graphics context
  gWindow =
      glfwCreateWindow(1280, 720, "Robot Simulation GUI", nullptr, nullptr);
  if (!gWindow) return false;
  glfwMakeContextCurrent(gWindow);
  glfwSwapInterval(1);  // Enable vsync

  // Initialize OpenGL loader
  if (gl3wInit() != 0) {
    wpi::errs() << "Failed to initialize OpenGL loader!\n";
    return false;
  }

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;

  // Setup Dear ImGui style
  // ImGui::StyleColorsDark();
  ImGui::StyleColorsClassic();

  // Setup Platform/Renderer bindings
  ImGui_ImplGlfw_InitForOpenGL(gWindow, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  // Load Fonts
  // - If no fonts are loaded, dear imgui will use the default font. You can
  // also load multiple fonts and use ImGui::PushFont()/PopFont() to select
  // them.
  // - AddFontFromFileTTF() will return the ImFont* so you can store it if you
  // need to select the font among multiple.
  // - If the file cannot be loaded, the function will return NULL. Please
  // handle those errors in your application (e.g. use an assertion, or display
  // an error and quit).
  // - The fonts will be rasterized at a given size (w/ oversampling) and stored
  // into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which
  // ImGui_ImplXXXX_NewFrame below will call.
  // - Read 'misc/fonts/README.txt' for more instructions and details.
  // - Remember that in C/C++ if you want to include a backslash \ in a string
  // literal you need to write a double backslash \\ !
  // io.Fonts->AddFontDefault();
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
  // ImFont* font =
  // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f,
  // NULL, io.Fonts->GetGlyphRangesJapanese()); IM_ASSERT(font != NULL);

  for (auto&& initialize : gInitializers) {
    if (initialize) initialize();
  }

  return true;
}

void HALSimGui::Main(void*) {
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  // Main loop
  while (!glfwWindowShouldClose(gWindow) && !gExit) {
    // Poll and handle events (inputs, window resize, etc.)
    glfwPollEvents();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    for (auto&& execute : gExecutors) {
      if (execute) execute();
    }

    {
      ImGui::BeginMainMenuBar();

      if (ImGui::BeginMenu("Window")) {
        for (auto&& window : gWindows)
          ImGui::MenuItem(window.name.c_str(), nullptr, &window.visible);
        ImGui::EndMenu();
      }

      for (auto&& menu : gMenus) {
        if (menu) menu();
      }

#if 0
      char str[64];
      std::snprintf(str, sizeof(str), "%.3f ms/frame (%.1f FPS)",
                    1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);
      ImGui::SameLine(ImGui::GetWindowWidth() - ImGui::CalcTextSize(str).x -
                      10);
      ImGui::Text("%s", str);
#endif
      ImGui::EndMainMenuBar();
    }

    for (auto&& window : gWindows) {
      if (window.display && window.visible) {
        if (ImGui::Begin(window.name.c_str(), &window.visible, window.flags))
          window.display();
        ImGui::End();
      }
    }

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(gWindow, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(gWindow);
  }

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(gWindow);
  glfwTerminate();
}

void HALSimGui::Exit(void*) { gExit = true; }

extern "C" {

void HALSIMGUI_Add(void* param, void (*initialize)(void*)) {
  if (initialize) HALSimGui::Add([=] { initialize(param); });
}

void HALSIMGUI_AddExecute(void* param, void (*execute)(void*)) {
  if (execute) HALSimGui::AddExecute([=] { execute(param); });
}

void HALSIMGUI_AddWindow(const char* name, void* param, void (*display)(void*),
                         int32_t flags) {
  if (display) HALSimGui::AddWindow(name, [=] { display(param); }, flags);
}

void HALSIMGUI_AddMainMenu(void* param, void (*menu)(void*)) {
  if (menu) HALSimGui::AddMainMenu([=] { menu(param); });
}

}  // extern "C"

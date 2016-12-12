#include "DSWrapper.h"

#include "llvm/SmallString.h"

#include "HAL/cpp/priority_mutex.h"

priority_mutex joystickMutex;

struct Joystick {
  bool isHookedUp = false;
  float axes[12] {0};

  int buttonCount = 0;
  int axisCount = 0;
  uint32_t buttons = 0;
  uint32_t outputs = 0;
  uint32_t leftRumble = 0;
  uint32_t rightRumble = 0;
  llvm::SmallString<256> name {""};
};

static Joystick joysticks[6];

void StartDsLoop() {

}

void SetJoystickHookedUp(int joystick, bool isHookedUp) {

}

void SetJoystickButtonCount(int joystick, int count) {

}

void SetJoystickAxisCount(int joystick, int count) {

}

void SetJoystickPOVCount(int joystick, int count) {

}

void PutJoystickAxis(int joystick, int axis, float val) {

}

void PutJoystickPOV(int joystick, int povNum, int val) {

}

void PutJoystickButton(int joystick, int button, bool value) {

}

void PutJoystickButtons(int joystick, uint32_t value) {

}

void PutJoystickName(int joystick, const char* name) {
  std::lock_guard<priority_mutex> lock(joystickMutex);
  joysticks[joystick].name = llvm::StringRef(name, std::min(255, (int)strlen(name)));
}

uint32_t GetJoystickOutputs(int joystick) {
  std::lock_guard<priority_mutex> lock(joystickMutex);
  return joysticks[joystick].outputs;
}

void GetJoystickRumble(int joystick, int* left, int* right) {

}
#pragma once

#include <stdint.h>

bool StartDsLoop();

void SetJoystickHookedUp(int joystick, bool isHookedUp);

void SetJoystickButtonCount(int joystick, int count);

void SetJoystickAxisCount(int joystick, int count);

void SetJoystickPOVCount(int joystick, int count);

void PutJoystickAxis(int joystick, int axis, float val);

void PutJoystickPOV(int joystick, int povNum, int val);

void PutJoystickButton(int joystick, int button, bool value);

void PutJoystickButtons(int joystick, uint32_t value);

void PutJoystickName(int joystick, const char* name);

uint32_t GetJoystickOutputs(int joystick);

void GetJoystickRumble(int joystick, int* left, int* right);



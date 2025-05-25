// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

#include "frc/simulation/GamepadSim.h"

#include "frc/Gamepad.h"

using namespace frc;
using namespace frc::sim;

GamepadSim::GamepadSim(const Gamepad& joystick)
    : GenericHIDSim{joystick} {
  SetAxisCount(6);
  SetButtonCount(26);
  SetPOVCount(1);
}

GamepadSim::GamepadSim(int port) : GenericHIDSim{port} {
  SetAxisCount(6);
  SetButtonCount(26);
  SetPOVCount(1);
}

void GamepadSim::SetLeftX(double value) {
  SetRawAxis(Gamepad::Axis::kLeftX, value);
}

void GamepadSim::SetRightX(double value) {
  SetRawAxis(Gamepad::Axis::kRightX, value);
}

void GamepadSim::SetLeftY(double value) {
  SetRawAxis(Gamepad::Axis::kLeftY, value);
}

void GamepadSim::SetRightY(double value) {
  SetRawAxis(Gamepad::Axis::kRightY, value);
}

void GamepadSim::SetLeftTriggerAxis(double value) {
  SetRawAxis(Gamepad::Axis::kLeftTrigger, value);
}

void GamepadSim::SetRightTriggerAxis(double value) {
  SetRawAxis(Gamepad::Axis::kRightTrigger, value);
}

void GamepadSim::SetSouthButton(bool value) {
  SetRawButton(Gamepad::Button::kSouth, value);
}

void GamepadSim::SetEastButton(bool value) {
  SetRawButton(Gamepad::Button::kEast, value);
}

void GamepadSim::SetWestButton(bool value) {
  SetRawButton(Gamepad::Button::kWest, value);
}

void GamepadSim::SetNorthButton(bool value) {
  SetRawButton(Gamepad::Button::kNorth, value);
}

void GamepadSim::SetBackButton(bool value) {
  SetRawButton(Gamepad::Button::kBack, value);
}

void GamepadSim::SetGuideButton(bool value) {
  SetRawButton(Gamepad::Button::kGuide, value);
}

void GamepadSim::SetStartButton(bool value) {
  SetRawButton(Gamepad::Button::kStart, value);
}

void GamepadSim::SetLeftStickButton(bool value) {
  SetRawButton(Gamepad::Button::kLeftStick, value);
}

void GamepadSim::SetRightStickButton(bool value) {
  SetRawButton(Gamepad::Button::kRightStick, value);
}

void GamepadSim::SetLeftShoulderButton(bool value) {
  SetRawButton(Gamepad::Button::kLeftShoulder, value);
}

void GamepadSim::SetRightShoulderButton(bool value) {
  SetRawButton(Gamepad::Button::kRightShoulder, value);
}

void GamepadSim::SetDpadUpButton(bool value) {
  SetRawButton(Gamepad::Button::kDpadUp, value);
}

void GamepadSim::SetDpadDownButton(bool value) {
  SetRawButton(Gamepad::Button::kDpadDown, value);
}

void GamepadSim::SetDpadLeftButton(bool value) {
  SetRawButton(Gamepad::Button::kDpadLeft, value);
}

void GamepadSim::SetDpadRightButton(bool value) {
  SetRawButton(Gamepad::Button::kDpadRight, value);
}

void GamepadSim::SetMisc1Button(bool value) {
  SetRawButton(Gamepad::Button::kMisc1, value);
}

void GamepadSim::SetRightPaddle1Button(bool value) {
  SetRawButton(Gamepad::Button::kRightPaddle1, value);
}

void GamepadSim::SetLeftPaddle1Button(bool value) {
  SetRawButton(Gamepad::Button::kLeftPaddle1, value);
}

void GamepadSim::SetRightPaddle2Button(bool value) {
  SetRawButton(Gamepad::Button::kRightPaddle2, value);
}

void GamepadSim::SetLeftPaddle2Button(bool value) {
  SetRawButton(Gamepad::Button::kLeftPaddle2, value);
}

void GamepadSim::SetTouchpadButton(bool value) {
  SetRawButton(Gamepad::Button::kTouchpad, value);
}

void GamepadSim::SetMisc2Button(bool value) {
  SetRawButton(Gamepad::Button::kMisc2, value);
}

void GamepadSim::SetMisc3Button(bool value) {
  SetRawButton(Gamepad::Button::kMisc3, value);
}

void GamepadSim::SetMisc4Button(bool value) {
  SetRawButton(Gamepad::Button::kMisc4, value);
}

void GamepadSim::SetMisc5Button(bool value) {
  SetRawButton(Gamepad::Button::kMisc5, value);
}

void GamepadSim::SetMisc6Button(bool value) {
  SetRawButton(Gamepad::Button::kMisc6, value);
}

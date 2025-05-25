// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

#pragma once

#include "frc/simulation/GenericHIDSim.h"

namespace frc {

class Gamepad;

namespace sim {

/**
 * Class to control a simulated Gamepad controller.
 */
class GamepadSim : public GenericHIDSim {
 public:
  /**
   * Constructs from a Gamepad object.
   *
   * @param joystick controller to simulate
   */
  explicit GamepadSim(const Gamepad& joystick);

  /**
   * Constructs from a joystick port number.
   *
   * @param port port number
   */
  explicit GamepadSim(int port);

  /**
   * Change the left X value of the controller's joystick.
   *
   * @param value the new value
   */
  void SetLeftX(double value);

  /**
   * Change the right X value of the controller's joystick.
   *
   * @param value the new value
   */
  void SetRightX(double value);

  /**
   * Change the left Y value of the controller's joystick.
   *
   * @param value the new value
   */
  void SetLeftY(double value);

  /**
   * Change the right Y value of the controller's joystick.
   *
   * @param value the new value
   */
  void SetRightY(double value);

  /**
   * Change the value of the Left Trigger axis on the controller.
   *
   * @param value the new value
   */
  void SetLeftTriggerAxis(double value);

  /**
   * Change the value of the Right Trigger axis on the controller.
   *
   * @param value the new value
   */
  void SetRightTriggerAxis(double value);

  /**
   * Change the value of the South button on the controller.
   *
   * @param value the new value
   */
  void SetSouthButton(bool value);

  /**
   * Change the value of the East button on the controller.
   *
   * @param value the new value
   */
  void SetEastButton(bool value);

  /**
   * Change the value of the West button on the controller.
   *
   * @param value the new value
   */
  void SetWestButton(bool value);

  /**
   * Change the value of the North button on the controller.
   *
   * @param value the new value
   */
  void SetNorthButton(bool value);

  /**
   * Change the value of the Back button on the controller.
   *
   * @param value the new value
   */
  void SetBackButton(bool value);

  /**
   * Change the value of the Guide button on the controller.
   *
   * @param value the new value
   */
  void SetGuideButton(bool value);

  /**
   * Change the value of the Start button on the controller.
   *
   * @param value the new value
   */
  void SetStartButton(bool value);

  /**
   * Change the value of the Left Stick button on the controller.
   *
   * @param value the new value
   */
  void SetLeftStickButton(bool value);

  /**
   * Change the value of the Right Stick button on the controller.
   *
   * @param value the new value
   */
  void SetRightStickButton(bool value);

  /**
   * Change the value of the Left Shoulder button on the controller.
   *
   * @param value the new value
   */
  void SetLeftShoulderButton(bool value);

  /**
   * Change the value of the Right Shoulder button on the controller.
   *
   * @param value the new value
   */
  void SetRightShoulderButton(bool value);

  /**
   * Change the value of the DPad Up button on the controller.
   *
   * @param value the new value
   */
  void SetDpadUpButton(bool value);

  /**
   * Change the value of the DPad Down button on the controller.
   *
   * @param value the new value
   */
  void SetDpadDownButton(bool value);

  /**
   * Change the value of the DPad Left button on the controller.
   *
   * @param value the new value
   */
  void SetDpadLeftButton(bool value);

  /**
   * Change the value of the DPad Right button on the controller.
   *
   * @param value the new value
   */
  void SetDpadRightButton(bool value);

  /**
   * Change the value of the Misc 1 button on the controller.
   *
   * @param value the new value
   */
  void SetMisc1Button(bool value);

  /**
   * Change the value of the Right Paddle 1 button on the controller.
   *
   * @param value the new value
   */
  void SetRightPaddle1Button(bool value);

  /**
   * Change the value of the Left Paddle 1 button on the controller.
   *
   * @param value the new value
   */
  void SetLeftPaddle1Button(bool value);

  /**
   * Change the value of the Right Paddle 2 button on the controller.
   *
   * @param value the new value
   */
  void SetRightPaddle2Button(bool value);

  /**
   * Change the value of the Left Paddle 2 button on the controller.
   *
   * @param value the new value
   */
  void SetLeftPaddle2Button(bool value);

  /**
   * Change the value of the Touchpad button on the controller.
   *
   * @param value the new value
   */
  void SetTouchpadButton(bool value);

  /**
   * Change the value of the Misc 2 button on the controller.
   *
   * @param value the new value
   */
  void SetMisc2Button(bool value);

  /**
   * Change the value of the Misc 3 button on the controller.
   *
   * @param value the new value
   */
  void SetMisc3Button(bool value);

  /**
   * Change the value of the Misc 4 button on the controller.
   *
   * @param value the new value
   */
  void SetMisc4Button(bool value);

  /**
   * Change the value of the Misc 5 button on the controller.
   *
   * @param value the new value
   */
  void SetMisc5Button(bool value);

  /**
   * Change the value of the Misc 6 button on the controller.
   *
   * @param value the new value
   */
  void SetMisc6Button(bool value);

};

}  // namespace sim
}  // namespace frc

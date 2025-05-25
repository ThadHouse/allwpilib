// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

package edu.wpi.first.wpilibj.simulation;

import edu.wpi.first.wpilibj.Gamepad;

/** Class to control a simulated Gamepad controller. */
public class GamepadSim extends GenericHIDSim {
  /**
   * Constructs from a Gamepad object.
   *
   * @param joystick controller to simulate
   */
  @SuppressWarnings("this-escape")
  public GamepadSim(Gamepad joystick) {
    super(joystick);
    setAxisCount(6);
    setButtonCount(26);
    setPOVCount(1);
  }

  /**
   * Constructs from a joystick port number.
   *
   * @param port port number
   */
  @SuppressWarnings("this-escape")
  public GamepadSim(int port) {
    super(port);
    setAxisCount(6);
    setButtonCount(26);
    setPOVCount(1);
  }

  /**
   * Change the left X value of the controller's joystick.
   *
   * @param value the new value
   */
  public void setLeftX(double value) {
    setRawAxis(Gamepad.Axis.kLeftX.value, value);
  }

  /**
   * Change the right X value of the controller's joystick.
   *
   * @param value the new value
   */
  public void setRightX(double value) {
    setRawAxis(Gamepad.Axis.kRightX.value, value);
  }

  /**
   * Change the left Y value of the controller's joystick.
   *
   * @param value the new value
   */
  public void setLeftY(double value) {
    setRawAxis(Gamepad.Axis.kLeftY.value, value);
  }

  /**
   * Change the right Y value of the controller's joystick.
   *
   * @param value the new value
   */
  public void setRightY(double value) {
    setRawAxis(Gamepad.Axis.kRightY.value, value);
  }

  /**
   * Change the value of the Left Trigger axis on the controller.
   *
   * @param value the new value
   */
  public void setLeftTriggerAxis(double value) {
    setRawAxis(Gamepad.Axis.kLeftTrigger.value, value);
  }

  /**
   * Change the value of the Right Trigger axis on the controller.
   *
   * @param value the new value
   */
  public void setRightTriggerAxis(double value) {
    setRawAxis(Gamepad.Axis.kRightTrigger.value, value);
  }

  /**
   * Change the value of the South button on the controller.
   *
   * @param value the new value
   */
  public void setSouthButton(boolean value) {
    setRawButton(Gamepad.Button.kSouth.value, value);
  }

  /**
   * Change the value of the East button on the controller.
   *
   * @param value the new value
   */
  public void setEastButton(boolean value) {
    setRawButton(Gamepad.Button.kEast.value, value);
  }

  /**
   * Change the value of the West button on the controller.
   *
   * @param value the new value
   */
  public void setWestButton(boolean value) {
    setRawButton(Gamepad.Button.kWest.value, value);
  }

  /**
   * Change the value of the North button on the controller.
   *
   * @param value the new value
   */
  public void setNorthButton(boolean value) {
    setRawButton(Gamepad.Button.kNorth.value, value);
  }

  /**
   * Change the value of the Back button on the controller.
   *
   * @param value the new value
   */
  public void setBackButton(boolean value) {
    setRawButton(Gamepad.Button.kBack.value, value);
  }

  /**
   * Change the value of the Guide button on the controller.
   *
   * @param value the new value
   */
  public void setGuideButton(boolean value) {
    setRawButton(Gamepad.Button.kGuide.value, value);
  }

  /**
   * Change the value of the Start button on the controller.
   *
   * @param value the new value
   */
  public void setStartButton(boolean value) {
    setRawButton(Gamepad.Button.kStart.value, value);
  }

  /**
   * Change the value of the Left Stick button on the controller.
   *
   * @param value the new value
   */
  public void setLeftStickButton(boolean value) {
    setRawButton(Gamepad.Button.kLeftStick.value, value);
  }

  /**
   * Change the value of the Right Stick button on the controller.
   *
   * @param value the new value
   */
  public void setRightStickButton(boolean value) {
    setRawButton(Gamepad.Button.kRightStick.value, value);
  }

  /**
   * Change the value of the Left Shoulder button on the controller.
   *
   * @param value the new value
   */
  public void setLeftShoulderButton(boolean value) {
    setRawButton(Gamepad.Button.kLeftShoulder.value, value);
  }

  /**
   * Change the value of the Right Shoulder button on the controller.
   *
   * @param value the new value
   */
  public void setRightShoulderButton(boolean value) {
    setRawButton(Gamepad.Button.kRightShoulder.value, value);
  }

  /**
   * Change the value of the DPad Up button on the controller.
   *
   * @param value the new value
   */
  public void setDpadUpButton(boolean value) {
    setRawButton(Gamepad.Button.kDpadUp.value, value);
  }

  /**
   * Change the value of the DPad Down button on the controller.
   *
   * @param value the new value
   */
  public void setDpadDownButton(boolean value) {
    setRawButton(Gamepad.Button.kDpadDown.value, value);
  }

  /**
   * Change the value of the DPad Left button on the controller.
   *
   * @param value the new value
   */
  public void setDpadLeftButton(boolean value) {
    setRawButton(Gamepad.Button.kDpadLeft.value, value);
  }

  /**
   * Change the value of the DPad Right button on the controller.
   *
   * @param value the new value
   */
  public void setDpadRightButton(boolean value) {
    setRawButton(Gamepad.Button.kDpadRight.value, value);
  }

  /**
   * Change the value of the Misc 1 button on the controller.
   *
   * @param value the new value
   */
  public void setMisc1Button(boolean value) {
    setRawButton(Gamepad.Button.kMisc1.value, value);
  }

  /**
   * Change the value of the Right Paddle 1 button on the controller.
   *
   * @param value the new value
   */
  public void setRightPaddle1Button(boolean value) {
    setRawButton(Gamepad.Button.kRightPaddle1.value, value);
  }

  /**
   * Change the value of the Left Paddle 1 button on the controller.
   *
   * @param value the new value
   */
  public void setLeftPaddle1Button(boolean value) {
    setRawButton(Gamepad.Button.kLeftPaddle1.value, value);
  }

  /**
   * Change the value of the Right Paddle 2 button on the controller.
   *
   * @param value the new value
   */
  public void setRightPaddle2Button(boolean value) {
    setRawButton(Gamepad.Button.kRightPaddle2.value, value);
  }

  /**
   * Change the value of the Left Paddle 2 button on the controller.
   *
   * @param value the new value
   */
  public void setLeftPaddle2Button(boolean value) {
    setRawButton(Gamepad.Button.kLeftPaddle2.value, value);
  }

  /**
   * Change the value of the Touchpad button on the controller.
   *
   * @param value the new value
   */
  public void setTouchpadButton(boolean value) {
    setRawButton(Gamepad.Button.kTouchpad.value, value);
  }

  /**
   * Change the value of the Misc 2 button on the controller.
   *
   * @param value the new value
   */
  public void setMisc2Button(boolean value) {
    setRawButton(Gamepad.Button.kMisc2.value, value);
  }

  /**
   * Change the value of the Misc 3 button on the controller.
   *
   * @param value the new value
   */
  public void setMisc3Button(boolean value) {
    setRawButton(Gamepad.Button.kMisc3.value, value);
  }

  /**
   * Change the value of the Misc 4 button on the controller.
   *
   * @param value the new value
   */
  public void setMisc4Button(boolean value) {
    setRawButton(Gamepad.Button.kMisc4.value, value);
  }

  /**
   * Change the value of the Misc 5 button on the controller.
   *
   * @param value the new value
   */
  public void setMisc5Button(boolean value) {
    setRawButton(Gamepad.Button.kMisc5.value, value);
  }

  /**
   * Change the value of the Misc 6 button on the controller.
   *
   * @param value the new value
   */
  public void setMisc6Button(boolean value) {
    setRawButton(Gamepad.Button.kMisc6.value, value);
  }
}

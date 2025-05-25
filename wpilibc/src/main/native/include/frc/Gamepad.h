// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

#pragma once

#include <wpi/sendable/Sendable.h>
#include <wpi/sendable/SendableHelper.h>

#include "frc/GenericHID.h"

namespace frc {

/**
 * Handle input from Gamepad controllers connected to the Driver Station.
 *
 * This class handles Gamepad input that comes from the Driver Station. Each
 * time a value is requested the most recent value is returned. There is a
 * single class instance for each controller and the mapping of ports to
 * hardware buttons depends on the code in the Driver Station.
 *
 * Only first party controllers from  are guaranteed to have the
 * correct mapping, and only through the official NI DS. Sim is not guaranteed
 * to have the same mapping, as well as any 3rd party controllers.
 */
class Gamepad : public GenericHID,
                                    public wpi::Sendable,
                                    public wpi::SendableHelper<Gamepad> {
 public:
  /**
   * Construct an instance of a controller.
   *
   * The controller index is the USB port on the Driver Station.
   *
   * @param port The port on the Driver Station that the controller is plugged
   *             into (0-5).
   */
  explicit Gamepad(int port);

  ~Gamepad() override = default;

  Gamepad(Gamepad&&) = default;
  Gamepad& operator=(Gamepad&&) = default;

  /**
   * Get the X axis value of left side of the controller. Right is positive.
   *
   * @return the axis value.
   */
  double GetLeftX() const;

  /**
   * Get the X axis value of right side of the controller. Right is positive.
   *
   * @return the axis value.
   */
  double GetRightX() const;

  /**
   * Get the Y axis value of left side of the controller. Back is positive.
   *
   * @return the axis value.
   */
  double GetLeftY() const;

  /**
   * Get the Y axis value of right side of the controller. Back is positive.
   *
   * @return the axis value.
   */
  double GetRightY() const;

  /**
   * Get the Left Trigger axis value of the controller. Note that this axis
   * is bound to the range of [0, 1] as opposed to the usual [-1, 1].
   *
   * @return the axis value.
   */
  double GetLeftTriggerAxis() const;

  /**
   * Constructs an event instance around the axis value of the Left Trigger.
   * The returned trigger will be true when the axis value is greater than
   * {@code threshold}.
   * @param threshold the minimum axis value for the returned event to be true.
   * This value should be in the range [0, 1] where 0 is the unpressed state of
   * the axis.
   * @param loop the event loop instance to attach the event to.
   * @return an event instance that is true when the Left Trigger's axis
   * exceeds the provided threshold, attached to the given event loop
   */
  BooleanEvent LeftTrigger(double threshold, EventLoop* loop) const;

  /**
   * Constructs an event instance around the axis value of the Left Trigger.
   * The returned trigger will be true when the axis value is greater than 0.5.
   * @param loop the event loop instance to attach the event to.
   * @return an event instance that is true when the Left Trigger's axis
   * exceeds 0.5, attached to the given event loop
   */
  BooleanEvent LeftTrigger(EventLoop* loop) const;

  /**
   * Get the Right Trigger axis value of the controller. Note that this axis
   * is bound to the range of [0, 1] as opposed to the usual [-1, 1].
   *
   * @return the axis value.
   */
  double GetRightTriggerAxis() const;

  /**
   * Constructs an event instance around the axis value of the Right Trigger.
   * The returned trigger will be true when the axis value is greater than
   * {@code threshold}.
   * @param threshold the minimum axis value for the returned event to be true.
   * This value should be in the range [0, 1] where 0 is the unpressed state of
   * the axis.
   * @param loop the event loop instance to attach the event to.
   * @return an event instance that is true when the Right Trigger's axis
   * exceeds the provided threshold, attached to the given event loop
   */
  BooleanEvent RightTrigger(double threshold, EventLoop* loop) const;

  /**
   * Constructs an event instance around the axis value of the Right Trigger.
   * The returned trigger will be true when the axis value is greater than 0.5.
   * @param loop the event loop instance to attach the event to.
   * @return an event instance that is true when the Right Trigger's axis
   * exceeds 0.5, attached to the given event loop
   */
  BooleanEvent RightTrigger(EventLoop* loop) const;

  /**
   * Read the value of the South button on the controller.
   *
   * @return The state of the button.
   */
  bool GetSouthButton() const;

  /**
   * Whether the South button was pressed since the last check.
   *
   * @return Whether the button was pressed since the last check.
   */
  bool GetSouthButtonPressed();

  /**
   * Whether the South button was released since the last check.
   *
   * @return Whether the button was released since the last check.
   */
  bool GetSouthButtonReleased();

  /**
   * Constructs an event instance around the South button's
   * digital signal.
   *
   * @param loop the event loop instance to attach the event to.
   * @return an event instance representing the South button's
   * digital signal attached to the given loop.
   */
  BooleanEvent South(EventLoop* loop) const;

  /**
   * Read the value of the East button on the controller.
   *
   * @return The state of the button.
   */
  bool GetEastButton() const;

  /**
   * Whether the East button was pressed since the last check.
   *
   * @return Whether the button was pressed since the last check.
   */
  bool GetEastButtonPressed();

  /**
   * Whether the East button was released since the last check.
   *
   * @return Whether the button was released since the last check.
   */
  bool GetEastButtonReleased();

  /**
   * Constructs an event instance around the East button's
   * digital signal.
   *
   * @param loop the event loop instance to attach the event to.
   * @return an event instance representing the East button's
   * digital signal attached to the given loop.
   */
  BooleanEvent East(EventLoop* loop) const;

  /**
   * Read the value of the West button on the controller.
   *
   * @return The state of the button.
   */
  bool GetWestButton() const;

  /**
   * Whether the West button was pressed since the last check.
   *
   * @return Whether the button was pressed since the last check.
   */
  bool GetWestButtonPressed();

  /**
   * Whether the West button was released since the last check.
   *
   * @return Whether the button was released since the last check.
   */
  bool GetWestButtonReleased();

  /**
   * Constructs an event instance around the West button's
   * digital signal.
   *
   * @param loop the event loop instance to attach the event to.
   * @return an event instance representing the West button's
   * digital signal attached to the given loop.
   */
  BooleanEvent West(EventLoop* loop) const;

  /**
   * Read the value of the North button on the controller.
   *
   * @return The state of the button.
   */
  bool GetNorthButton() const;

  /**
   * Whether the North button was pressed since the last check.
   *
   * @return Whether the button was pressed since the last check.
   */
  bool GetNorthButtonPressed();

  /**
   * Whether the North button was released since the last check.
   *
   * @return Whether the button was released since the last check.
   */
  bool GetNorthButtonReleased();

  /**
   * Constructs an event instance around the North button's
   * digital signal.
   *
   * @param loop the event loop instance to attach the event to.
   * @return an event instance representing the North button's
   * digital signal attached to the given loop.
   */
  BooleanEvent North(EventLoop* loop) const;

  /**
   * Read the value of the Back button on the controller.
   *
   * @return The state of the button.
   */
  bool GetBackButton() const;

  /**
   * Whether the Back button was pressed since the last check.
   *
   * @return Whether the button was pressed since the last check.
   */
  bool GetBackButtonPressed();

  /**
   * Whether the Back button was released since the last check.
   *
   * @return Whether the button was released since the last check.
   */
  bool GetBackButtonReleased();

  /**
   * Constructs an event instance around the Back button's
   * digital signal.
   *
   * @param loop the event loop instance to attach the event to.
   * @return an event instance representing the Back button's
   * digital signal attached to the given loop.
   */
  BooleanEvent Back(EventLoop* loop) const;

  /**
   * Read the value of the Guide button on the controller.
   *
   * @return The state of the button.
   */
  bool GetGuideButton() const;

  /**
   * Whether the Guide button was pressed since the last check.
   *
   * @return Whether the button was pressed since the last check.
   */
  bool GetGuideButtonPressed();

  /**
   * Whether the Guide button was released since the last check.
   *
   * @return Whether the button was released since the last check.
   */
  bool GetGuideButtonReleased();

  /**
   * Constructs an event instance around the Guide button's
   * digital signal.
   *
   * @param loop the event loop instance to attach the event to.
   * @return an event instance representing the Guide button's
   * digital signal attached to the given loop.
   */
  BooleanEvent Guide(EventLoop* loop) const;

  /**
   * Read the value of the Start button on the controller.
   *
   * @return The state of the button.
   */
  bool GetStartButton() const;

  /**
   * Whether the Start button was pressed since the last check.
   *
   * @return Whether the button was pressed since the last check.
   */
  bool GetStartButtonPressed();

  /**
   * Whether the Start button was released since the last check.
   *
   * @return Whether the button was released since the last check.
   */
  bool GetStartButtonReleased();

  /**
   * Constructs an event instance around the Start button's
   * digital signal.
   *
   * @param loop the event loop instance to attach the event to.
   * @return an event instance representing the Start button's
   * digital signal attached to the given loop.
   */
  BooleanEvent Start(EventLoop* loop) const;

  /**
   * Read the value of the Left Stick button on the controller.
   *
   * @return The state of the button.
   */
  bool GetLeftStickButton() const;

  /**
   * Whether the Left Stick button was pressed since the last check.
   *
   * @return Whether the button was pressed since the last check.
   */
  bool GetLeftStickButtonPressed();

  /**
   * Whether the Left Stick button was released since the last check.
   *
   * @return Whether the button was released since the last check.
   */
  bool GetLeftStickButtonReleased();

  /**
   * Constructs an event instance around the Left Stick button's
   * digital signal.
   *
   * @param loop the event loop instance to attach the event to.
   * @return an event instance representing the Left Stick button's
   * digital signal attached to the given loop.
   */
  BooleanEvent LeftStick(EventLoop* loop) const;

  /**
   * Read the value of the Right Stick button on the controller.
   *
   * @return The state of the button.
   */
  bool GetRightStickButton() const;

  /**
   * Whether the Right Stick button was pressed since the last check.
   *
   * @return Whether the button was pressed since the last check.
   */
  bool GetRightStickButtonPressed();

  /**
   * Whether the Right Stick button was released since the last check.
   *
   * @return Whether the button was released since the last check.
   */
  bool GetRightStickButtonReleased();

  /**
   * Constructs an event instance around the Right Stick button's
   * digital signal.
   *
   * @param loop the event loop instance to attach the event to.
   * @return an event instance representing the Right Stick button's
   * digital signal attached to the given loop.
   */
  BooleanEvent RightStick(EventLoop* loop) const;

  /**
   * Read the value of the Left Shoulder button on the controller.
   *
   * @return The state of the button.
   */
  bool GetLeftShoulderButton() const;

  /**
   * Whether the Left Shoulder button was pressed since the last check.
   *
   * @return Whether the button was pressed since the last check.
   */
  bool GetLeftShoulderButtonPressed();

  /**
   * Whether the Left Shoulder button was released since the last check.
   *
   * @return Whether the button was released since the last check.
   */
  bool GetLeftShoulderButtonReleased();

  /**
   * Constructs an event instance around the Left Shoulder button's
   * digital signal.
   *
   * @param loop the event loop instance to attach the event to.
   * @return an event instance representing the Left Shoulder button's
   * digital signal attached to the given loop.
   */
  BooleanEvent LeftShoulder(EventLoop* loop) const;

  /**
   * Read the value of the Right Shoulder button on the controller.
   *
   * @return The state of the button.
   */
  bool GetRightShoulderButton() const;

  /**
   * Whether the Right Shoulder button was pressed since the last check.
   *
   * @return Whether the button was pressed since the last check.
   */
  bool GetRightShoulderButtonPressed();

  /**
   * Whether the Right Shoulder button was released since the last check.
   *
   * @return Whether the button was released since the last check.
   */
  bool GetRightShoulderButtonReleased();

  /**
   * Constructs an event instance around the Right Shoulder button's
   * digital signal.
   *
   * @param loop the event loop instance to attach the event to.
   * @return an event instance representing the Right Shoulder button's
   * digital signal attached to the given loop.
   */
  BooleanEvent RightShoulder(EventLoop* loop) const;

  /**
   * Read the value of the DPad Up button on the controller.
   *
   * @return The state of the button.
   */
  bool GetDpadUpButton() const;

  /**
   * Whether the DPad Up button was pressed since the last check.
   *
   * @return Whether the button was pressed since the last check.
   */
  bool GetDpadUpButtonPressed();

  /**
   * Whether the DPad Up button was released since the last check.
   *
   * @return Whether the button was released since the last check.
   */
  bool GetDpadUpButtonReleased();

  /**
   * Constructs an event instance around the DPad Up button's
   * digital signal.
   *
   * @param loop the event loop instance to attach the event to.
   * @return an event instance representing the DPad Up button's
   * digital signal attached to the given loop.
   */
  BooleanEvent DpadUp(EventLoop* loop) const;

  /**
   * Read the value of the DPad Down button on the controller.
   *
   * @return The state of the button.
   */
  bool GetDpadDownButton() const;

  /**
   * Whether the DPad Down button was pressed since the last check.
   *
   * @return Whether the button was pressed since the last check.
   */
  bool GetDpadDownButtonPressed();

  /**
   * Whether the DPad Down button was released since the last check.
   *
   * @return Whether the button was released since the last check.
   */
  bool GetDpadDownButtonReleased();

  /**
   * Constructs an event instance around the DPad Down button's
   * digital signal.
   *
   * @param loop the event loop instance to attach the event to.
   * @return an event instance representing the DPad Down button's
   * digital signal attached to the given loop.
   */
  BooleanEvent DpadDown(EventLoop* loop) const;

  /**
   * Read the value of the DPad Left button on the controller.
   *
   * @return The state of the button.
   */
  bool GetDpadLeftButton() const;

  /**
   * Whether the DPad Left button was pressed since the last check.
   *
   * @return Whether the button was pressed since the last check.
   */
  bool GetDpadLeftButtonPressed();

  /**
   * Whether the DPad Left button was released since the last check.
   *
   * @return Whether the button was released since the last check.
   */
  bool GetDpadLeftButtonReleased();

  /**
   * Constructs an event instance around the DPad Left button's
   * digital signal.
   *
   * @param loop the event loop instance to attach the event to.
   * @return an event instance representing the DPad Left button's
   * digital signal attached to the given loop.
   */
  BooleanEvent DpadLeft(EventLoop* loop) const;

  /**
   * Read the value of the DPad Right button on the controller.
   *
   * @return The state of the button.
   */
  bool GetDpadRightButton() const;

  /**
   * Whether the DPad Right button was pressed since the last check.
   *
   * @return Whether the button was pressed since the last check.
   */
  bool GetDpadRightButtonPressed();

  /**
   * Whether the DPad Right button was released since the last check.
   *
   * @return Whether the button was released since the last check.
   */
  bool GetDpadRightButtonReleased();

  /**
   * Constructs an event instance around the DPad Right button's
   * digital signal.
   *
   * @param loop the event loop instance to attach the event to.
   * @return an event instance representing the DPad Right button's
   * digital signal attached to the given loop.
   */
  BooleanEvent DpadRight(EventLoop* loop) const;

  /**
   * Read the value of the Misc 1 button on the controller.
   *
   * @return The state of the button.
   */
  bool GetMisc1Button() const;

  /**
   * Whether the Misc 1 button was pressed since the last check.
   *
   * @return Whether the button was pressed since the last check.
   */
  bool GetMisc1ButtonPressed();

  /**
   * Whether the Misc 1 button was released since the last check.
   *
   * @return Whether the button was released since the last check.
   */
  bool GetMisc1ButtonReleased();

  /**
   * Constructs an event instance around the Misc 1 button's
   * digital signal.
   *
   * @param loop the event loop instance to attach the event to.
   * @return an event instance representing the Misc 1 button's
   * digital signal attached to the given loop.
   */
  BooleanEvent Misc1(EventLoop* loop) const;

  /**
   * Read the value of the Right Paddle 1 button on the controller.
   *
   * @return The state of the button.
   */
  bool GetRightPaddle1Button() const;

  /**
   * Whether the Right Paddle 1 button was pressed since the last check.
   *
   * @return Whether the button was pressed since the last check.
   */
  bool GetRightPaddle1ButtonPressed();

  /**
   * Whether the Right Paddle 1 button was released since the last check.
   *
   * @return Whether the button was released since the last check.
   */
  bool GetRightPaddle1ButtonReleased();

  /**
   * Constructs an event instance around the Right Paddle 1 button's
   * digital signal.
   *
   * @param loop the event loop instance to attach the event to.
   * @return an event instance representing the Right Paddle 1 button's
   * digital signal attached to the given loop.
   */
  BooleanEvent RightPaddle1(EventLoop* loop) const;

  /**
   * Read the value of the Left Paddle 1 button on the controller.
   *
   * @return The state of the button.
   */
  bool GetLeftPaddle1Button() const;

  /**
   * Whether the Left Paddle 1 button was pressed since the last check.
   *
   * @return Whether the button was pressed since the last check.
   */
  bool GetLeftPaddle1ButtonPressed();

  /**
   * Whether the Left Paddle 1 button was released since the last check.
   *
   * @return Whether the button was released since the last check.
   */
  bool GetLeftPaddle1ButtonReleased();

  /**
   * Constructs an event instance around the Left Paddle 1 button's
   * digital signal.
   *
   * @param loop the event loop instance to attach the event to.
   * @return an event instance representing the Left Paddle 1 button's
   * digital signal attached to the given loop.
   */
  BooleanEvent LeftPaddle1(EventLoop* loop) const;

  /**
   * Read the value of the Right Paddle 2 button on the controller.
   *
   * @return The state of the button.
   */
  bool GetRightPaddle2Button() const;

  /**
   * Whether the Right Paddle 2 button was pressed since the last check.
   *
   * @return Whether the button was pressed since the last check.
   */
  bool GetRightPaddle2ButtonPressed();

  /**
   * Whether the Right Paddle 2 button was released since the last check.
   *
   * @return Whether the button was released since the last check.
   */
  bool GetRightPaddle2ButtonReleased();

  /**
   * Constructs an event instance around the Right Paddle 2 button's
   * digital signal.
   *
   * @param loop the event loop instance to attach the event to.
   * @return an event instance representing the Right Paddle 2 button's
   * digital signal attached to the given loop.
   */
  BooleanEvent RightPaddle2(EventLoop* loop) const;

  /**
   * Read the value of the Left Paddle 2 button on the controller.
   *
   * @return The state of the button.
   */
  bool GetLeftPaddle2Button() const;

  /**
   * Whether the Left Paddle 2 button was pressed since the last check.
   *
   * @return Whether the button was pressed since the last check.
   */
  bool GetLeftPaddle2ButtonPressed();

  /**
   * Whether the Left Paddle 2 button was released since the last check.
   *
   * @return Whether the button was released since the last check.
   */
  bool GetLeftPaddle2ButtonReleased();

  /**
   * Constructs an event instance around the Left Paddle 2 button's
   * digital signal.
   *
   * @param loop the event loop instance to attach the event to.
   * @return an event instance representing the Left Paddle 2 button's
   * digital signal attached to the given loop.
   */
  BooleanEvent LeftPaddle2(EventLoop* loop) const;

  /**
   * Read the value of the Touchpad button on the controller.
   *
   * @return The state of the button.
   */
  bool GetTouchpadButton() const;

  /**
   * Whether the Touchpad button was pressed since the last check.
   *
   * @return Whether the button was pressed since the last check.
   */
  bool GetTouchpadButtonPressed();

  /**
   * Whether the Touchpad button was released since the last check.
   *
   * @return Whether the button was released since the last check.
   */
  bool GetTouchpadButtonReleased();

  /**
   * Constructs an event instance around the Touchpad button's
   * digital signal.
   *
   * @param loop the event loop instance to attach the event to.
   * @return an event instance representing the Touchpad button's
   * digital signal attached to the given loop.
   */
  BooleanEvent Touchpad(EventLoop* loop) const;

  /**
   * Read the value of the Misc 2 button on the controller.
   *
   * @return The state of the button.
   */
  bool GetMisc2Button() const;

  /**
   * Whether the Misc 2 button was pressed since the last check.
   *
   * @return Whether the button was pressed since the last check.
   */
  bool GetMisc2ButtonPressed();

  /**
   * Whether the Misc 2 button was released since the last check.
   *
   * @return Whether the button was released since the last check.
   */
  bool GetMisc2ButtonReleased();

  /**
   * Constructs an event instance around the Misc 2 button's
   * digital signal.
   *
   * @param loop the event loop instance to attach the event to.
   * @return an event instance representing the Misc 2 button's
   * digital signal attached to the given loop.
   */
  BooleanEvent Misc2(EventLoop* loop) const;

  /**
   * Read the value of the Misc 3 button on the controller.
   *
   * @return The state of the button.
   */
  bool GetMisc3Button() const;

  /**
   * Whether the Misc 3 button was pressed since the last check.
   *
   * @return Whether the button was pressed since the last check.
   */
  bool GetMisc3ButtonPressed();

  /**
   * Whether the Misc 3 button was released since the last check.
   *
   * @return Whether the button was released since the last check.
   */
  bool GetMisc3ButtonReleased();

  /**
   * Constructs an event instance around the Misc 3 button's
   * digital signal.
   *
   * @param loop the event loop instance to attach the event to.
   * @return an event instance representing the Misc 3 button's
   * digital signal attached to the given loop.
   */
  BooleanEvent Misc3(EventLoop* loop) const;

  /**
   * Read the value of the Misc 4 button on the controller.
   *
   * @return The state of the button.
   */
  bool GetMisc4Button() const;

  /**
   * Whether the Misc 4 button was pressed since the last check.
   *
   * @return Whether the button was pressed since the last check.
   */
  bool GetMisc4ButtonPressed();

  /**
   * Whether the Misc 4 button was released since the last check.
   *
   * @return Whether the button was released since the last check.
   */
  bool GetMisc4ButtonReleased();

  /**
   * Constructs an event instance around the Misc 4 button's
   * digital signal.
   *
   * @param loop the event loop instance to attach the event to.
   * @return an event instance representing the Misc 4 button's
   * digital signal attached to the given loop.
   */
  BooleanEvent Misc4(EventLoop* loop) const;

  /**
   * Read the value of the Misc 5 button on the controller.
   *
   * @return The state of the button.
   */
  bool GetMisc5Button() const;

  /**
   * Whether the Misc 5 button was pressed since the last check.
   *
   * @return Whether the button was pressed since the last check.
   */
  bool GetMisc5ButtonPressed();

  /**
   * Whether the Misc 5 button was released since the last check.
   *
   * @return Whether the button was released since the last check.
   */
  bool GetMisc5ButtonReleased();

  /**
   * Constructs an event instance around the Misc 5 button's
   * digital signal.
   *
   * @param loop the event loop instance to attach the event to.
   * @return an event instance representing the Misc 5 button's
   * digital signal attached to the given loop.
   */
  BooleanEvent Misc5(EventLoop* loop) const;

  /**
   * Read the value of the Misc 6 button on the controller.
   *
   * @return The state of the button.
   */
  bool GetMisc6Button() const;

  /**
   * Whether the Misc 6 button was pressed since the last check.
   *
   * @return Whether the button was pressed since the last check.
   */
  bool GetMisc6ButtonPressed();

  /**
   * Whether the Misc 6 button was released since the last check.
   *
   * @return Whether the button was released since the last check.
   */
  bool GetMisc6ButtonReleased();

  /**
   * Constructs an event instance around the Misc 6 button's
   * digital signal.
   *
   * @param loop the event loop instance to attach the event to.
   * @return an event instance representing the Misc 6 button's
   * digital signal attached to the given loop.
   */
  BooleanEvent Misc6(EventLoop* loop) const;

  /** Represents a digital button on an Gamepad. */
  struct Button {
    /// South button.
    static constexpr int kSouth = 1;
    /// East button.
    static constexpr int kEast = 2;
    /// West button.
    static constexpr int kWest = 3;
    /// North button.
    static constexpr int kNorth = 4;
    /// Back button.
    static constexpr int kBack = 5;
    /// Guide button.
    static constexpr int kGuide = 6;
    /// Start button.
    static constexpr int kStart = 7;
    /// Left Stick button.
    static constexpr int kLeftStick = 8;
    /// Right Stick button.
    static constexpr int kRightStick = 9;
    /// Left Shoulder button.
    static constexpr int kLeftShoulder = 10;
    /// Right Shoulder button.
    static constexpr int kRightShoulder = 11;
    /// DPad Up button.
    static constexpr int kDpadUp = 12;
    /// DPad Down button.
    static constexpr int kDpadDown = 13;
    /// DPad Left button.
    static constexpr int kDpadLeft = 14;
    /// DPad Right button.
    static constexpr int kDpadRight = 15;
    /// Misc 1 button.
    static constexpr int kMisc1 = 16;
    /// Right Paddle 1 button.
    static constexpr int kRightPaddle1 = 17;
    /// Left Paddle 1 button.
    static constexpr int kLeftPaddle1 = 18;
    /// Right Paddle 2 button.
    static constexpr int kRightPaddle2 = 19;
    /// Left Paddle 2 button.
    static constexpr int kLeftPaddle2 = 20;
    /// Touchpad button.
    static constexpr int kTouchpad = 21;
    /// Misc 2 button.
    static constexpr int kMisc2 = 22;
    /// Misc 3 button.
    static constexpr int kMisc3 = 23;
    /// Misc 4 button.
    static constexpr int kMisc4 = 24;
    /// Misc 5 button.
    static constexpr int kMisc5 = 25;
    /// Misc 6 button.
    static constexpr int kMisc6 = 26;
  };

  /** Represents an axis on an Gamepad. */
  struct Axis {
    /// Left X axis.
    static constexpr int kLeftX = 0;
    /// Right X axis.
    static constexpr int kRightX = 3;
    /// Left Y axis.
    static constexpr int kLeftY = 1;
    /// Right Y axis.
    static constexpr int kRightY = 4;
    /// Left trigger.
    static constexpr int kLeftTrigger = 5;
    /// Right trigger.
    static constexpr int kRightTrigger = 6;
  };

  void InitSendable(wpi::SendableBuilder& builder) override;
};

}  // namespace frc

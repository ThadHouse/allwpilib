// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

package org.wpilib.drivers.motor;

import org.wpilib.hardware.hal.HALUtil;

/** REVLib-compatible error categories returned by high-level A301 commands. */
public enum A301Error {
  /** No error occurred. */
  OK(0),
  /** A general controller error occurred. */
  GENERAL_ERROR(1),
  /** The operation timed out. */
  TIMEOUT(2),
  /** The requested operation is not implemented. */
  NOT_IMPLEMENTED(3),
  /** The HAL returned an error. */
  HAL_ERROR(4),
  /** The controller firmware could not be read. */
  CANT_FIND_FIRMWARE(5),
  /** The controller firmware is too old. */
  FIRMWARE_TOO_OLD(6),
  /** The controller firmware is newer than this driver supports. */
  FIRMWARE_TOO_NEW(7),
  /** The parameter ID is invalid. */
  PARAM_INVALID_ID(8),
  /** The parameter type does not match the expected type. */
  PARAM_MISMATCH_TYPE(9),
  /** The parameter does not support the requested access mode. */
  PARAM_ACCESS_MODE(10),
  /** The parameter value is invalid. */
  PARAM_INVALID(11),
  /** The parameter is deprecated or not implemented. */
  PARAM_NOT_IMPLEMENTED_DEPRECATED(12),
  /** The follower configuration does not match its leader. */
  FOLLOW_CONFIG_MISMATCH(13),
  /** The returned error value is invalid. */
  INVALID(14),
  /** The requested setpoint is outside the accepted range. */
  SETPOINT_OUT_OF_RANGE(15),
  /** An unknown controller error occurred. */
  UNKNOWN(16),
  /** The controller is disconnected from the CAN bus. */
  CAN_DISCONNECTED(17),
  /** Another device is using the requested CAN ID. */
  DUPLICATE_CAN_ID(18),
  /** The requested CAN ID is invalid. */
  INVALID_CAN_ID(19);

  /** REVLib-compatible numeric value. */
  @SuppressWarnings("MemberName")
  public final int value;

  A301Error(int value) {
    this.value = value;
  }

  /**
   * Returns the error corresponding to a REVLib-compatible numeric value.
   *
   * @param value numeric error value
   * @return corresponding error, or {@link #INVALID} when unknown
   */
  public static A301Error fromInt(int value) {
    for (A301Error error : values()) {
      if (error.value == value) {
        return error;
      }
    }
    return INVALID;
  }

  static A301Error fromHalStatus(int status) {
    return switch (status) {
      case 0 -> OK;
      case -1154, -44087 -> TIMEOUT;
      case HALUtil.PARAMETER_OUT_OF_RANGE -> PARAM_INVALID;
      case -1029 -> DUPLICATE_CAN_ID;
      case -1030 -> INVALID_CAN_ID;
      case HALUtil.INCOMPATIBLE_STATE -> GENERAL_ERROR;
      case -35007 -> CAN_DISCONNECTED;
      default -> HAL_ERROR;
    };
  }
}

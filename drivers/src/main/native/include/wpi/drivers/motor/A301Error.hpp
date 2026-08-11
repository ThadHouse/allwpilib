// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

#pragma once

#include <stdint.h>

#include "wpi/hal/CAN.h"
#include "wpi/hal/Errors.h"

namespace wpi {

/** REVLib-compatible error categories returned by high-level A301 commands. */
enum class A301Error {
  /** No error occurred. */
  OK = 0,
  /** A general controller error occurred. */
  GENERAL_ERROR = 1,
  /** The operation timed out. */
  TIMEOUT = 2,
  /** The requested operation is not implemented. */
  NOT_IMPLEMENTED = 3,
  /** The HAL returned an error. */
  HAL_ERROR = 4,
  /** The controller firmware could not be read. */
  CANT_FIND_FIRMWARE = 5,
  /** The controller firmware is too old. */
  FIRMWARE_TOO_OLD = 6,
  /** The controller firmware is newer than this driver supports. */
  FIRMWARE_TOO_NEW = 7,
  /** The parameter ID is invalid. */
  PARAM_INVALID_ID = 8,
  /** The parameter type does not match the expected type. */
  PARAM_MISMATCH_TYPE = 9,
  /** The parameter does not support the requested access mode. */
  PARAM_ACCESS_MODE = 10,
  /** The parameter value is invalid. */
  PARAM_INVALID = 11,
  /** The parameter is deprecated or not implemented. */
  PARAM_NOT_IMPLEMENTED_DEPRECATED = 12,
  /** The follower configuration does not match its leader. */
  FOLLOW_CONFIG_MISMATCH = 13,
  /** The returned error value is invalid. */
  INVALID = 14,
  /** The requested setpoint is outside the accepted range. */
  SETPOINT_OUT_OF_RANGE = 15,
  /** An unknown controller error occurred. */
  UNKNOWN = 16,
  /** The controller is disconnected from the CAN bus. */
  CAN_DISCONNECTED = 17,
  /** Another device is using the requested CAN ID. */
  DUPLICATE_CAN_ID = 18,
  /** The requested CAN ID is invalid. */
  INVALID_CAN_ID = 19,
};

namespace detail {

/**
 * Converts a HAL status code to its REVLib-compatible A301 error category.
 *
 * @param status HAL status code
 * @return corresponding A301 error category
 */
constexpr A301Error A301ErrorFromHalStatus(int32_t status) {
  switch (status) {
    case 0:
      return A301Error::OK;
    case HAL_CAN_TIMEOUT:
    case HAL_ERR_CANSessionMux_MessageNotFound:
      return A301Error::TIMEOUT;
    case HAL_PARAMETER_OUT_OF_RANGE:
      return A301Error::PARAM_INVALID;
    case HAL_RESOURCE_IS_ALLOCATED:
      return A301Error::DUPLICATE_CAN_ID;
    case HAL_RESOURCE_OUT_OF_RANGE:
      return A301Error::INVALID_CAN_ID;
    case HAL_INCOMPATIBLE_STATE:
      return A301Error::GENERAL_ERROR;
    case HAL_CAN_BUFFER_OVERRUN:
      return A301Error::CAN_DISCONNECTED;
    default:
      return A301Error::HAL_ERROR;
  }
}

}  // namespace detail
}  // namespace wpi

/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2016-2017. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "HAL/Power.h"

#include <memory>

#include "HAL/ChipObject.h"

using namespace hal;

static std::unique_ptr<tPower> power;

static void initializePower(int32_t* status) {
  if (power == nullptr) {
    power.reset(tPower::create(status));
  }
}

extern "C" {

/**
 * Get the roboRIO input voltage
 */
double HAL_GetVinVoltage(int32_t* status) {
  initializePower(status);
  return power->readVinVoltage(status) / 4.096 * 0.025733 - 0.029;
}

/**
 * Get the roboRIO input current
 */
double HAL_GetVinCurrent(int32_t* status) {
  initializePower(status);
  return power->readVinCurrent(status) / 4.096 * 0.017042 - 0.071;
}

/**
 * Get the 6V rail voltage
 */
double HAL_GetUserVoltage6V(int32_t* status) {
  initializePower(status);
  return power->readUserVoltage6V(status) / 4.096 * 0.007019 - 0.014;
}

/**
 * Get the 6V rail current
 */
double HAL_GetUserCurrent6V(int32_t* status) {
  initializePower(status);
  return power->readUserCurrent6V(status) / 4.096 * 0.005566 - 0.009;
}

/**
 * Get the status of the 6V rail
 */
HAL_PowerStatus HAL_GetUserStatus6V(int32_t* status) {
  initializePower(status);
  return static_cast<HAL_PowerStatus(power->readStatus_User6V(status));
}

/**
 * Get the fault count for the 6V rail
 */
int32_t HAL_GetUserCurrentFaults6V(int32_t* status) {
  initializePower(status);
  return static_cast<int32_t>(
      power->readFaultCounts_OverCurrentFaultCount6V(status));
}

/**
 * Get the 5V rail voltage
 */
double HAL_GetUserVoltage5V(int32_t* status) {
  initializePower(status);
  return power->readUserVoltage5V(status) / 4.096 * 0.005962 - 0.013;
}

/**
 * Get the 5V rail current
 */
double HAL_GetUserCurrent5V(int32_t* status) {
  initializePower(status);
  return power->readUserCurrent5V(status) / 4.096 * 0.001996 - 0.002;
}

/**
 * Get the status of the 5V rail
 */
HAL_PowerStatus HAL_GetUserStatus5V(int32_t* status) {
  initializePower(status);
  return static_cast<HAL_PowerStatus(power->readStatus_User5V(status));
}

/**
 * Get the fault count for the 5V rail
 */
int32_t HAL_GetUserCurrentFaults5V(int32_t* status) {
  initializePower(status);
  return static_cast<int32_t>(
      power->readFaultCounts_OverCurrentFaultCount5V(status));
}

/**
 * Get the 3.3V rail voltage
 */
double HAL_GetUserVoltage3V3(int32_t* status) {
  initializePower(status);
  return power->readUserVoltage3V3(status) / 4.096 * 0.004902 - 0.01;
}

/**
 * Get the 3.3V rail current
 */
double HAL_GetUserCurrent3V3(int32_t* status) {
  initializePower(status);
  return power->readUserCurrent3V3(status) / 4.096 * 0.002486 - 0.003;
}

/**
 * Get the status of the 3.3V rail
 */
HAL_PowerStatus HAL_GetUserStatus3V3(int32_t* status) {
  initializePower(status);
  return static_cast<HAL_PowerStatus(power->readStatus_User3V3(status));
}

/**
 * Get the fault count for the 3.3V rail
 */
int32_t HAL_GetUserCurrentFaults3V3(int32_t* status) {
  initializePower(status);
  return static_cast<int32_t>(
      power->readFaultCounts_OverCurrentFaultCount3V3(status));
}

void HAL_SetUserVoltage3V3Disabled(HAL_Bool disable, int32_t* status) {
  initializePower(status);
  return 
}
void HAL_SetUserVoltage5VDisabled(HAL_Bool disable, int32_t* status);
void HAL_SetUserVoltage6VDisabled(HAL_Bool disable, int32_t* status);

}  // extern "C"

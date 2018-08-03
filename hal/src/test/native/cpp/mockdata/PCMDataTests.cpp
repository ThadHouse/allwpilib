/*----------------------------------------------------------------------------*/
/* Copyright (c) 2015-2018 FIRST. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "gtest/gtest.h"
#include "hal/HAL.h"
#include "hal/PCM.h"
#include "hal/handles/HandlesInternal.h"
#include "mockdata/PCMData.h"

namespace hal {

std::string gTestSolenoidCallbackName;
HAL_Value gTestSolenoidCallbackValue;

void TestSolenoidInitializationCallback(const char* name, void* param,
                                        const struct HAL_Value* value) {
  gTestSolenoidCallbackName = name;
  gTestSolenoidCallbackValue = *value;
}

TEST(SolenoidSimTests, TestPCMInitialization) {
  const int MODULE_TO_TEST = 2;
  const int CHANNEL_TO_TEST = 3;

  int callbackParam = 0;
  int callbackId = HALSIM_RegisterPCMInitializedCallback(
      MODULE_TO_TEST, &TestSolenoidInitializationCallback,
      &callbackParam, false);
  ASSERT_TRUE(0 != callbackId);

  int32_t status;
  int32_t module;
  HAL_PCMHandle solenoidHandle;

  // Use out of range index
  status = 0;
  module = 8000;
  gTestSolenoidCallbackName = "Unset";
  solenoidHandle = HAL_GetPCMHandle(module, &status);
  EXPECT_EQ(HAL_kInvalidHandle, solenoidHandle);
  EXPECT_EQ(HAL_HANDLE_ERROR, status);
  EXPECT_STREQ("Unset", gTestSolenoidCallbackName.c_str());

  // Successful setup
  status = 0;
  module = MODULE_TO_TEST;
  gTestSolenoidCallbackName = "Unset";
  solenoidHandle = HAL_GetPCMHandle(module, &status);
  EXPECT_TRUE(HAL_kInvalidHandle != solenoidHandle);
  EXPECT_EQ(0, status);
  EXPECT_STREQ("Initialized", gTestSolenoidCallbackName.c_str());

  // Reset, should allow you to re-register
  hal::HandleBase::ResetGlobalHandles();
  HALSIM_ResetPCMData(MODULE_TO_TEST);
  callbackId = HALSIM_RegisterPCMInitializedCallback(
      MODULE_TO_TEST, &TestSolenoidInitializationCallback,
      &callbackParam, false);
  ASSERT_TRUE(0 != callbackId);
}

}  // namespace hal

#include "CrossConnects.h"
#include "RAIIWrappers.h"

#include "llvm/StringRef.h"

#include "gtest/gtest.h"
#include "llvm/SmallVector.h"

#include "HAL/HAL.h"

#include "HAL/cpp/priority_mutex.h"
#include "HAL/cpp/priority_condition_variable.h"

#include "TestHelpers.h"

#define I2C_ADDRESS 4

class PDPTest : public ::testing::TestWithParam<int> {
};

INSTANTIATE_TEST_CASE_P(PDPTests, PDPTest,
                        ::testing::Values(0, 35, 52));

TEST(PDPTest, PDPCheckModuleRange) {
  for (int i = 0; i < HAL_GetNumPDPModules(); i++)
  {
    ASSERT_TRUE(HAL_CheckPDPModule(i));
  }
}

TEST(PDPTest, PDPCheckChannelRange) {
  for (int i = 0; i < HAL_GetNumPDPChannels(); i++)
  {
    ASSERT_TRUE(HAL_CheckPDPChannel(i));
  }
}

TEST(PDPTest, PDPCheckModuleOverUnderRange) {
  ASSERT_FALSE(HAL_CheckPDPModule(-1));
  ASSERT_FALSE(HAL_CheckPDPModule(HAL_GetNumPDPModules()));
}

TEST(PDPTest, PDPCheckChannelOverUnderRange) {
  ASSERT_FALSE(HAL_CheckPDPChannel(-1));
  ASSERT_FALSE(HAL_CheckPDPChannel(HAL_GetNumPDPChannels()));
}

TEST(PDPTest, PDPCheckModuleInitOverUnderRange) {
  int32_t status = 0;
  HAL_InitializePDP(-1, &status);
  ASSERT_NE(status, 0);
  status = 0;
  HAL_InitializePDP(HAL_GetNumPDPModules(), &status);
  ASSERT_NE(status, 0);
}

static int32_t TempToByte(double tempC)
{
  return (tempC + 67.8564500484966) / 1.03250836957542;
}


TEST_P(PDPTest, TestGetTemperature) {
  int32_t status = 0;
  HAL_InitializeI2C(0, &status);
  ASSERT_EQ(status, 0);


  int canID = GetParam();

  HAL_InitializePDP(canID, &status);
  ASSERT_EQ(status, 0);

  uint8_t sendBuffer[2 + 4 + 8];

  int arbId = 0x8041480 | canID;

  uint8_t data[8] = {0};
  data[7] = 0;
  FillCANBuffer(sendBuffer, data, arbId, 8);

  HAL_WriteI2C(0, I2C_ADDRESS, sendBuffer, 2 + 4 + 8);

  Wait(0.01);

  double temp = HAL_GetPDPTemperature(canID, &status);
  ASSERT_EQ(status, 0);
  ASSERT_NEAR(temp, -67.85, 0.2);

  double temps[5] = {-30.56, 0, 25.43, 42.21, 78.65};

  for (int i = 0; i < 5; i++) {
    data[7] = TempToByte(temps[i]);
  FillCANBuffer(sendBuffer, data, arbId, 8);

  HAL_WriteI2C(0, I2C_ADDRESS, sendBuffer, 2 + 4 + 8);

  Wait(0.01);

  double temp = HAL_GetPDPTemperature(canID, &status);
  ASSERT_EQ(status, 0);
  ASSERT_NEAR(temp, temps[i], 1.0);
  } 
}

static int32_t VoltageToByte(double busVolt)
{
  return (busVolt - 4.0) / 0.05;
}

TEST_P(PDPTest, TestGetVoltage) {
  int32_t status = 0;
  HAL_InitializeI2C(0, &status);
  ASSERT_EQ(status, 0);


  int canID = GetParam();

  HAL_InitializePDP(canID, &status);
  ASSERT_EQ(status, 0);

  uint8_t sendBuffer[2 + 4 + 8];

  int arbId = 0x8041480 | canID;

  uint8_t data[8] = {0};
  data[6] = 0;
  FillCANBuffer(sendBuffer, data, arbId, 8);

  HAL_WriteI2C(0, I2C_ADDRESS, sendBuffer, 2 + 4 + 8);

  Wait(0.01);

  double temp = HAL_GetPDPVoltage(canID, &status);
  ASSERT_EQ(status, 0);
  ASSERT_NEAR(temp, 4.0, 0.2);

  double temps[5] = {4.5, 5.6, 9.2, 12.4, 13.5};

  for (int i = 0; i < 5; i++) {
    data[6] = VoltageToByte(temps[i]);
  FillCANBuffer(sendBuffer, data, arbId, 8);

  HAL_WriteI2C(0, I2C_ADDRESS, sendBuffer, 2 + 4 + 8);

  Wait(0.01);

  double temp = HAL_GetPDPVoltage(canID, &status);
  ASSERT_EQ(status, 0);
  ASSERT_NEAR(temp, temps[i], 1.0);
  } 
}
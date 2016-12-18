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

class SolenoidTest : public ::testing::TestWithParam<int> {
};

INSTANTIATE_TEST_CASE_P(SolenoidPcmTests, SolenoidTest,
                        ::testing::Values(0, 35, 52));

TEST_P(SolenoidTest, TestSetAll)
{
  int32_t status = 0;
  HAL_InitializeI2C(0, &status);
  ASSERT_EQ(status, 0);

  int canID = GetParam();

  llvm::SmallVector<SolenoidHandle, 8> handles;

  for (int i = 0; i < 8; i++)
  {
    int32_t status = 0;
    handles.emplace_back(canID, i, &status);
    ASSERT_EQ(status, 0);
  }

  for (int i = 0; i < 8; i++) {
    int32_t status = 0;
    HAL_SetSolenoid(handles[i], false, &status);
    ASSERT_EQ(status, 0);
    }

  Wait(0.025);  // Wait 50 ms

  uint8_t data[2];
  data[0] = '1';
  data[1] = canID;
  // Write our seed data
  status = HAL_WriteI2C(0, I2C_ADDRESS, data, 2);
  ASSERT_EQ(status, 1);

  uint8_t buf[3];
  status = HAL_ReadI2C(0, I2C_ADDRESS, buf, 3);

  ASSERT_EQ(buf[0], '1');
  ASSERT_EQ(buf[1], canID);
  ASSERT_EQ(buf[2], 0);

  for (int i = 0; i < 8; i++)
  {
    int32_t status = 0;
    HAL_SetSolenoid(handles[i], true, &status);
    ASSERT_EQ(status, 0);
  }

  Wait(0.025);  // Wait 50 ms

  //uint8_t data[2];
  data[0] = '1';
  data[1] = canID;
  // Write our seed data
  status = HAL_WriteI2C(0, I2C_ADDRESS, data, 2);
  ASSERT_EQ(status, 1);

  //uint8_t buf[3];
  status = HAL_ReadI2C(0, I2C_ADDRESS, buf, 3);

  ASSERT_EQ(buf[0], '1');
  ASSERT_EQ(buf[1], canID);
  ASSERT_EQ(buf[2], 0xFF);

  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      int32_t status = 0;
      bool set = j == i;
      HAL_SetSolenoid(handles[j], set, &status);
      ASSERT_EQ(status, 0);
    }

    Wait(0.025);  // Wait 50 ms

    //uint8_t data[2];
    data[0] = '1';
    data[1] = canID;
    // Write our seed data
    status = HAL_WriteI2C(0, I2C_ADDRESS, data, 2);
    ASSERT_EQ(status, 1);

    //uint8_t buf[3];
    status = HAL_ReadI2C(0, I2C_ADDRESS, buf, 3);

    ASSERT_EQ(buf[0], '1');
    ASSERT_EQ(buf[1], canID);
    ASSERT_EQ(buf[2], 1 << i);
  } 
}

TEST_P(SolenoidTest, TestGetAll) {
  int32_t status = 0;
  HAL_InitializeI2C(0, &status);
  ASSERT_EQ(status, 0);

  int canID = GetParam();

  uint8_t data[2 + 4 + 8];

  data[0] = 'T';
  data[1] = 0;

  int arbId = 0x9041400 | canID;

  std::memcpy(&(data[2]), &arbId, 4);

  data[6] = 0;
  for (int i = 1; i < 8; i++) {
    data[6 + i] = 0;
  }

  HAL_WriteI2C(0, I2C_ADDRESS, data, 2 + 4 + 8;

  Wait(0.01);

  int32_t val = HAL_GetAllSolenoids(canID, &status);
  ASSERT_EQ(status, 0);
  ASSERT_EQ(val, 0);

  data[0] = 'T';
  data[1] = 0;

  std::memcpy(&(data[2]), &arbId, 4);

  data[6] = 0xff;
  for (int i = 1; i < 8; i++) {
    data[6 + i] = 0;
  }

  HAL_WriteI2C(0, I2C_ADDRESS, data, 2 + 4 + 8);

  Wait(0.01);

  val = HAL_GetAllSolenoids(canID, &status);
  ASSERT_EQ(status, 0);
  ASSERT_EQ(val, 0xff);

  for (int i = 0; i < 8; i++)
  {
    data[0] = 'T';
    data[1] = 0;

    std::memcpy(&(data[2]), &arbId, 4);

    data[6] = 1 << i;
    for (int i = 1; i < 8; i++) {
      data[6 + i] = 0;
    }

    HAL_WriteI2C(0, I2C_ADDRESS, data, 2 + 4 + 8);

    Wait(0.01);

    val = HAL_GetAllSolenoids(canID, &status);
    ASSERT_EQ(status, 0);
    ASSERT_EQ(val, 1 << i);
  } 
}
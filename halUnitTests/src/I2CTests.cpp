#include "CrossConnects.h"
#include "RAIIWrappers.h"

#include "llvm/StringRef.h"

#include "gtest/gtest.h"

#include "HAL/HAL.h"

#include "HAL/cpp/priority_mutex.h"
#include "HAL/cpp/priority_condition_variable.h"

#include <chrono>
#include <thread>

#define I2C_ADDRESS 4

const char* i2cTestData = "9Welcome Back!";

class I2CTest : public ::testing::TestWithParam<std::pair<int32_t, HAL_SerialPort>> {
};

INSTANTIATE_TEST_CASE_P(I2CSerialPortCrossConnectTests, I2CTest,
                        ::testing::ValuesIn(I2C_Serial_CrossConnects));

TEST_P(I2CTest, TestI2CRead) {
  auto param = GetParam();
  int32_t status = 0;
  HAL_InitializeI2C(param.first, &status);
  ASSERT_EQ(status, 0);

  // Write our seed data
  status = HAL_WriteI2C(param.first, I2C_ADDRESS, (uint8_t*)i2cTestData, strlen(i2cTestData));
  ASSERT_EQ(status, 1);

  uint8_t buf[strlen(i2cTestData) + 1];
  status = HAL_ReadI2C(param.first, I2C_ADDRESS, buf, strlen(i2cTestData));
  buf[strlen(i2cTestData)] = 0;

  ASSERT_EQ(status, 1);
  ASSERT_EQ(llvm::StringRef((char*)buf), llvm::StringRef(i2cTestData));
}

static void SetupSerialPort(HAL_SerialPort port, int32_t* status) {
  *status = 0;
  HAL_InitializeSerialPort(port, status);
  ASSERT_EQ(*status, 0);

  HAL_SetSerialBaudRate(port, 57600, status);
  ASSERT_EQ(*status, 0);
}

TEST_P(I2CTest, TestI2CWrite) { auto param = GetParam();
  int32_t status = 0;
  SetupSerialPort(param.second, &status);

  HAL_InitializeI2C(param.first, &status);
  ASSERT_EQ(status, 0);

  HAL_ClearSerial(param.second, &status);
  ASSERT_EQ(status, 0);

  status = HAL_WriteI2C(param.first, I2C_ADDRESS, (uint8_t*)i2cTestData, strlen(i2cTestData));
  ASSERT_EQ(status, 1);

  status = 0;

  char buf[strlen(i2cTestData) + 1];
  int32_t count =
      HAL_ReadSerial(param.second, buf, strlen(i2cTestData), &status);
  ASSERT_EQ(status, 0);
  ASSERT_EQ(count, (int32_t)strlen(i2cTestData));
  buf[strlen(i2cTestData)] = 0;
  ASSERT_EQ(llvm::StringRef(buf), llvm::StringRef(i2cTestData));




}
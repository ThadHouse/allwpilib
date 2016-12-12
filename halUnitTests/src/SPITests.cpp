#include "CrossConnects.h"
#include "RAIIWrappers.h"

#include "llvm/StringRef.h"

#include "gtest/gtest.h"

#include "HAL/HAL.h"

#include "HAL/cpp/priority_mutex.h"
#include "HAL/cpp/priority_condition_variable.h"

#include <chrono>
#include <thread>

const char* spiTestData = "Welcome Back!\n";
const char* spiRequest ="12\n";

class SPITest : public ::testing::TestWithParam<std::pair<int32_t, HAL_SerialPort>> {
};

INSTANTIATE_TEST_CASE_P(SPISerialPortCrossConnectTests, SPITest,
                        ::testing::ValuesIn(SPI_Serial_CrossConnects));

TEST_P(SPITest, TestSPIRead) {
  auto param = GetParam();
  int32_t status = 0;
  HAL_InitializeSPI(param.first, &status);
  ASSERT_EQ(status, 0);

  status = HAL_WriteSPI(param.first, (uint8_t*)spiRequest, strlen(spiRequest));
  ASSERT_EQ(status, (int)strlen(spiRequest));

  status = 0;

  uint8_t buf[strlen(spiTestData) + 1];
  status = HAL_ReadSPI(param.first, buf, strlen(spiTestData));
  buf[strlen(spiTestData)] = 0;

  ASSERT_EQ(status, (int32_t)strlen(spiTestData));
  ASSERT_EQ(llvm::StringRef((char*)buf, strlen(spiTestData)), llvm::StringRef(spiTestData));
}

static void SetupSerialPort(HAL_SerialPort port, int32_t* status) {
  *status = 0;
  HAL_InitializeSerialPort(port, status);
  ASSERT_EQ(*status, 0);

  HAL_SetSerialBaudRate(port, 57600, status);
  ASSERT_EQ(*status, 0);
}

TEST_P(SPITest, TestSPIWrite) { auto param = GetParam();
  int32_t status = 0;
  SetupSerialPort(param.second, &status);

  HAL_InitializeSPI(param.first, &status);
  ASSERT_EQ(status, 0);

  HAL_ClearSerial(param.second, &status);
  ASSERT_EQ(status, 0);

  status = HAL_WriteSPI(param.first, (uint8_t*)spiTestData, strlen(spiTestData));
  ASSERT_EQ(status, (int)strlen(spiTestData));

  status = 0;

  char buf[strlen(spiTestData) + 1];
  int32_t count =
      HAL_ReadSerial(param.second, buf, strlen(spiTestData), &status);
  ASSERT_EQ(status, 0);
  ASSERT_EQ(count, (int32_t)strlen(spiTestData));
  //buf[strlen(spiTestData) - 1] = '\n';
  buf[strlen(spiTestData)] = 0;
  ASSERT_EQ(llvm::StringRef(buf), llvm::StringRef(spiTestData));




}
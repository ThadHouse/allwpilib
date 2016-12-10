#include "llvm/SmallVector.h"

#include "gtest/gtest.h"

#include "HAL/HAL.h"

#include "HAL/cpp/priority_mutex.h"
#include "HAL/cpp/priority_condition_variable.h"

using namespace hal;

TEST(HALAPI, GetPort) { 
  auto handle1 = HAL_GetPort(0);
  auto handle2 = HAL_GetPort(36);

  ASSERT_TRUE(isHandleType(handle1, HAL_HandleEnum::Port));
  ASSERT_TRUE(isHandleType(handle2, HAL_HandleEnum::Port));
  ASSERT_EQ(getPortHandleChannel(handle1), 0);
  ASSERT_EQ(getPortHandleChannel(handle2), 33);
  ASSERT_EQ(getPortHandleModule(handle1), 0);
  ASSERT_EQ(getPortHandleModule(handle2), 0);
}

TEST(HALAPI, GetPortModule) { 
  auto handle1 = HAL_GetPort(0, 35);
  auto handle2 = HAL_GetPort(36, 0);

  ASSERT_TRUE(isHandleType(handle1, HAL_HandleEnum::Port));
  ASSERT_TRUE(isHandleType(handle2, HAL_HandleEnum::Port));
  ASSERT_EQ(getPortHandleChannel(handle1), 0);
  ASSERT_EQ(getPortHandleChannel(handle2), 33);
  ASSERT_EQ(getPortHandleModule(handle1), 35);
  ASSERT_EQ(getPortHandleModule(handle2), 0);
}


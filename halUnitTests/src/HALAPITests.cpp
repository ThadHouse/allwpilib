#include "llvm/SmallVector.h"

#include "gtest/gtest.h"

#include "HAL/HAL.h"
#include "HAL/handles/HandlesInternal.h"

#include "HAL/cpp/priority_mutex.h"
#include "HAL/cpp/priority_condition_variable.h"

using namespace hal;

TEST(HALAPI, GetPort) { 
  auto handle1 = HAL_GetPort(0);
  auto handle2 = HAL_GetPort(36);

  ASSERT_TRUE(isHandleType(handle1, HAL_HandleEnum::Port));
  ASSERT_TRUE(isHandleType(handle2, HAL_HandleEnum::Port));
  ASSERT_EQ(getPortHandleChannel(handle1), 0);
  ASSERT_EQ(getPortHandleChannel(handle2), 36);
  ASSERT_EQ(getPortHandleModule(handle1), 1);
  ASSERT_EQ(getPortHandleModule(handle2), 1);
}

TEST(HALAPI, GetPortModule) { 
  auto handle1 = HAL_GetPortWithModule(35, 0);
  auto handle2 = HAL_GetPortWithModule(0, 36);

  ASSERT_TRUE(isHandleType(handle1, HAL_HandleEnum::Port));
  ASSERT_TRUE(isHandleType(handle2, HAL_HandleEnum::Port));
  ASSERT_EQ(getPortHandleChannel(handle1), 0);
  ASSERT_EQ(getPortHandleChannel(handle2), 36);
  ASSERT_EQ(getPortHandleModule(handle1), 35);
  ASSERT_EQ(getPortHandleModule(handle2), 0);
}


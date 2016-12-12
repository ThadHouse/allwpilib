#include "CrossConnects.h"
#include "RAIIWrappers.h"

#include "llvm/SmallVector.h"

#include "gtest/gtest.h"

#include "HAL/HAL.h"

#include "HAL/cpp/priority_mutex.h"
#include "HAL/cpp/priority_condition_variable.h"

class DIOTest : public ::testing::TestWithParam<CrossConnectPair> {
};

INSTANTIATE_TEST_CASE_P(DIOCrossConnectTests, DIOTest,
                        ::testing::ValuesIn(DIO_DIO_CrossConnects));

TEST(DIOTest, TestAllocateAll) {
  llvm::SmallVector<DIOHandle, 32> dioHandles;
  for (int i = 0; i < HAL_GetNumDigitalChannels(); i++) {
    int32_t status = 0;
    dioHandles.emplace_back(DIOHandle(i, true, &status));
    ASSERT_EQ(status, 0);
  }
}

TEST(DIOTest, TestMultipleAllocateFails) {
  int32_t status = 0;
  DIOHandle handle(0, true, &status);
  ASSERT_NE(handle, HAL_kInvalidHandle);
  ASSERT_EQ(status, 0);

  DIOHandle handle2(0, true, &status);
  ASSERT_EQ(handle2, HAL_kInvalidHandle);
  ASSERT_EQ(status, RESOURCE_IS_ALLOCATED);
}

TEST(DIOTest, TestOverAllocateFails) {
  int32_t status = 0;
  DIOHandle handle(HAL_GetNumDigitalChannels(), true, &status);
  ASSERT_EQ(handle, HAL_kInvalidHandle);
  ASSERT_EQ(status, PARAMETER_OUT_OF_RANGE);
}

TEST(DIOTest, TestUnderAllocateFails) {
  int32_t status = 0;
  DIOHandle handle(-1, true, &status);
  ASSERT_EQ(handle, HAL_kInvalidHandle);
  ASSERT_EQ(status, PARAMETER_OUT_OF_RANGE);
}

TEST(DIOTest, TestCrossAllocationFails) {
  int32_t status = 0;
  PWMHandle pwmHandle(10, &status);
  ASSERT_NE(pwmHandle, HAL_kInvalidHandle);
  ASSERT_EQ(status, 0);
  DIOHandle handle(10, true, &status);
  ASSERT_EQ(handle, HAL_kInvalidHandle);
  ASSERT_EQ(status, RESOURCE_IS_ALLOCATED);
}
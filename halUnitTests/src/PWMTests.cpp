#include "CrossConnects.h"
#include "RAIIWrappers.h"

#include "llvm/SmallVector.h"

#include "gtest/gtest.h"

#include "HAL/HAL.h"

#include "HAL/cpp/priority_mutex.h"
#include "HAL/cpp/priority_condition_variable.h"



class PWMTest : public ::testing::TestWithParam<CrossConnectPair> {
};

struct InterruptCheckData {
  llvm::SmallVector<double, 8> risingStamps;
  llvm::SmallVector<double, 8> fallingStamps;
  priority_mutex mutex;
  priority_condition_variable cond;
  HAL_InterruptHandle handle;
};

void TestTiming(int squelch, CrossConnectPair param) {
  // Initialize interrupt
  int32_t status = 0;
  InterruptHandle interruptHandle(false, &status);
  // Ensure we have a valid interrupt handle
  ASSERT_NE(interruptHandle, HAL_kInvalidHandle);

  status = 0;
  PWMHandle pwmHandle(param.first, &status);
  ASSERT_NE(pwmHandle, HAL_kInvalidHandle);

  // Ensure our PWM is disabled, and set up properly
  HAL_SetPWMRaw(pwmHandle, 0, &status);
  HAL_SetPWMConfig(pwmHandle, 2.0, 1.0, 1.0, 0, 0, &status);
  HAL_SetPWMPeriodScale(pwmHandle, squelch, &status);

  unsigned int checkPeriod = 0;
  switch (squelch) {
    case (0):
      checkPeriod = 5050;
      break;
      case (1):
        checkPeriod = 10100;
        break;
        case (3):
          checkPeriod = 20200;
          break;
  }

  status = 0;
  DIOHandle dioHandle(param.second, true, &status);
  ASSERT_NE(dioHandle, HAL_kInvalidHandle);

  InterruptCheckData interruptData;
  interruptData.handle = interruptHandle;

  // Can use any type for the interrupt handle
  HAL_RequestInterrupts(interruptHandle, dioHandle,
                        HAL_AnalogTriggerType::HAL_Trigger_kInWindow, &status);

  HAL_SetInterruptUpSourceEdge(interruptHandle, true, true, &status);

  status = 0;
  HAL_AttachInterruptHandler(
      interruptHandle,
      [](uint32_t mask, void* param) {
        InterruptCheckData* data = static_cast<InterruptCheckData*>(param);
        if ((mask & 0x100) == 0x100 && data->risingStamps.size() == 0 &&
            data->fallingStamps.size() == 0) {
          // Falling edge at start of tracking. Skip
          return;
        }

        if (data->fallingStamps.size() == 4) {
          // We've finished, return
          return;
        }

        int32_t status = 0;
        if ((mask & 0x1) == 0x1) {
          // Rising Edge
          data->risingStamps.push_back(
              HAL_ReadInterruptRisingTimestamp(data->handle, &status));
        } else if ((mask & 0x100) == 0x100) {
          // Falling Edge
          data->fallingStamps.push_back(
              HAL_ReadInterruptFallingTimestamp(data->handle, &status));
        }

        if (data->risingStamps.size() == 4 && data->fallingStamps.size() == 4) {
          // Kill Interrupts
          HAL_DisableInterrupts(data->handle, &status);
          data->cond.notify_all();
        }
      },
      &interruptData, &status);

  // Loop to test 5 speeds
  for (unsigned int i = 1000; i < 2100; i += 250) {
    interruptData.risingStamps.clear();
    interruptData.fallingStamps.clear();
    status = 0;
    HAL_EnableInterrupts(interruptHandle, &status);

    // Ensure our interrupt actually got created correctly.
    ASSERT_EQ(status, 0);
    HAL_SetPWMSpeed(pwmHandle, (i - 1000) / 1000.0, &status);
    ASSERT_EQ(status, 0);
    { 
      std::unique_lock<priority_mutex> lock(interruptData.mutex);
      // Wait for lock
      // TODO: Add Timeout
      interruptData.cond.wait(lock);
    }
    // Needed for timeout condition
    HAL_DisableInterrupts(interruptHandle, &status);

    HAL_SetPWMRaw(pwmHandle, 0, &status);

    // Ensure our interrupts have the proper counts
    ASSERT_EQ(interruptData.risingStamps.size(),
              interruptData.fallingStamps.size());

    ASSERT_EQ(interruptData.risingStamps.size() % 2, (unsigned int)0);
    ASSERT_EQ(interruptData.fallingStamps.size() % 2, (unsigned int)0);

    for (unsigned int j = 0; j < interruptData.risingStamps.size(); j++) {
      unsigned int width = ((int)(interruptData.fallingStamps[j] / 1e-6) -
                    (int)(interruptData.risingStamps[j] / 1e-6));
      j++;
      ASSERT_NEAR(width, i, 10);
    }

    for (unsigned int j = 0; j < interruptData.risingStamps.size(); j++) {
      unsigned int period = ((int)(interruptData.risingStamps[j + 1] / 1e-6) -
                    (int)(interruptData.risingStamps[j] / 1e-6));
      j++;
      ASSERT_NEAR(period, checkPeriod, 10);
    }
  }
}

TEST_P(PWMTest, TestTiming4x) { 
  auto param = GetParam();
  TestTiming(3, param);
}

TEST_P(PWMTest, TestTiming2x) { 
  auto param = GetParam();
  TestTiming(1, param);
}

TEST_P(PWMTest, TestTiming1x) { 
  auto param = GetParam();
  TestTiming(0, param);
}

TEST(PWMTest, TestAllocateAll) {
  llvm::SmallVector<PWMHandle, 21> pwmHandles;
  for (int i = 0; i < HAL_GetNumPWMChannels(); i++) {
    int32_t status = 0;
    pwmHandles.emplace_back(PWMHandle(i, &status));
    ASSERT_EQ(status, 0);
  }
}

TEST(PWMTest, TestMultipleAllocateFails) {
  int32_t status = 0;
  PWMHandle handle(0, &status);
  ASSERT_NE(handle, HAL_kInvalidHandle);
  ASSERT_EQ(status, 0);

  PWMHandle handle2(0, &status);
  ASSERT_EQ(handle2, HAL_kInvalidHandle);
  ASSERT_EQ(status, RESOURCE_IS_ALLOCATED);
}

TEST(PWMTest, TestOverAllocateFails) {
  int32_t status = 0;
  PWMHandle handle(HAL_GetNumPWMChannels(), &status);
  ASSERT_EQ(handle, HAL_kInvalidHandle);
  ASSERT_EQ(status, PARAMETER_OUT_OF_RANGE);
}

TEST(PWMTest, TestUnderAllocateFails) {
  int32_t status = 0;
  PWMHandle handle(-1, &status);
  ASSERT_EQ(handle, HAL_kInvalidHandle);
  ASSERT_EQ(status, PARAMETER_OUT_OF_RANGE);
}

TEST(PWMTest, TestCrossAllocationFails) {
  int32_t status = 0;
  DIOHandle dioHandle(10, true, &status);
  ASSERT_NE(dioHandle, HAL_kInvalidHandle);
  ASSERT_EQ(status, 0);
  PWMHandle handle(10, &status);
  ASSERT_EQ(handle, HAL_kInvalidHandle);
  ASSERT_EQ(status, RESOURCE_IS_ALLOCATED);
}

//INSTANTIATE_TEST_CASE_P(PWMCrossConnectTests, PWMTest,
  //                      ::testing::ValuesIn(PWM_DIO_CrossConnects));
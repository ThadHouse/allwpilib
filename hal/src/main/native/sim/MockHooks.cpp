/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2017. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include <atomic>
#include <chrono>
#include <cstdio>
#include <thread>

#include "MockHooksInternal.h"
#include "support/timestamp.h"

static std::atomic<bool> programStarted{false};

static std::atomic<uint64_t> programStartTime{0};

namespace hal {
void RestartTiming() { programStartTime = wpi::Now() / 10; }

int64_t GetFPGATime() {
  auto now = wpi::Now() / 10;
  auto currentTime = now - programStartTime;
  return currentTime;
}

double GetFPGATimestamp() {
  auto now = wpi::Now() / 10;
  auto currentTime = now - programStartTime;
  return currentTime * 1.0e-6;
}

void SetProgramStarted() { programStarted = true; }
}  // namespace hal

using namespace hal;

extern "C" {
void HALSIM_WaitForProgramStart(void) {
  int count = 0;
  while (!programStarted) {
    count++;
    std::printf("Waiting for program start signal: %d\n", count);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
}

void HALSIM_SetProgramStarted(void) { SetProgramStarted(); }

void HALSIM_RestartTiming(void) { RestartTiming(); }
}

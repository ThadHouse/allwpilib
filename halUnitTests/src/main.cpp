// Copyright 2006, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <stdio.h>
#include "gtest/gtest.h"

#include <chrono>
#include <thread>
#include <atomic>

#include "HAL/HAL.h"
using namespace std::chrono_literals;

GTEST_API_ int main(int argc, char **argv) {
  printf("Starting HAL\n");
  if (!HAL_Initialize(0)) {
    printf("FATAL ERROR: HAL could not be initialized\n");
    return -1;
  }

  // Wait for driver station to enable
  HAL_ObserveUserProgramStarting();
  printf("Waiting for Enable\n");

  std::atomic_bool keepRunning{true};

  // Fire up a thread to keep robot enabled
  std::thread thread([&keepRunning]() {
    while (keepRunning) {
      HAL_ObserveUserProgramTeleop();
      std::this_thread::sleep_for(500ms);
    }
  });

  int count = 0;
  while (true) {
    HAL_ControlWord word;
    HAL_GetControlWord(&word);
    if (word.enabled) break;
    count++;
    printf("Wait for enable count %d\n", count);
    std::this_thread::sleep_for(100ms);
  }

  printf("Running main() from gtest_main.cc\n");
  testing::InitGoogleTest(&argc, argv);
  auto ret = RUN_ALL_TESTS();
  keepRunning = false;
  thread.join();
  return ret;
}

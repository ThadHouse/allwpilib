#include "TestHelpers.h"

#include <chrono>
#include <thread>
#include <cstring>

void Wait(double seconds) {
  if (seconds < 0.0) return;
  std::this_thread::sleep_for(std::chrono::duration<double>(seconds));
}

void FillCANBuffer(uint8_t* sendArray, uint8_t* data, int arbId, int count) {
  sendArray[0] = 'T';
  sendArray[1] = 0;

  std::memcpy(&(sendArray[2]), &arbId, 4);
  for (int i = 0; i < count; i++) {
    sendArray[6 + i] = data[i];
  }
}
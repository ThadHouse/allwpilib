#include <chrono>
#include <thread>

void Wait(double seconds) {
  if (seconds < 0.0) return;
  std::this_thread::sleep_for(std::chrono::duration<double>(seconds));
}
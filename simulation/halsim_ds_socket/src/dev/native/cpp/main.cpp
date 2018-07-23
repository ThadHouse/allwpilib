#include "HAL/HAL.h"
#include <thread>
#include <wpi/raw_ostream.h>

extern "C" int HALSIM_InitExtension(void);



int main() {
  HAL_Initialize(500,0);
  HALSIM_InitExtension();

  wpi::outs() << "Hello, starting sleep loop\n";
  wpi::outs().flush();
  while (true) {
    std::this_thread::sleep_for(std::chrono::seconds(1000));
  }
  wpi::outs() << "Left sleep loop?\n";
  wpi::outs().flush();
}

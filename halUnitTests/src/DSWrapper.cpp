#include "DSWrapper.h"

#include "llvm/SmallString.h"
#include "llvm/raw_ostream.h"
#include "HAL/cpp/priority_mutex.h"
#include "HAL/Notifier.h"
#include "HAL/HAL.h"

#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>

#include <thread>

priority_mutex joystickMutex;

struct Joystick {
  bool isHookedUp = false;
  float axes[12] {0};

  int buttonCount = 0;
  int axisCount = 0;
  uint32_t buttons = 0;
  uint32_t outputs = 0;
  uint32_t leftRumble = 0;
  uint32_t rightRumble = 0;
  llvm::SmallString<256> name {""};
};

static Joystick joysticks[6];

static HAL_NotifierHandle sendThreadHandle;
static std::thread receiveThread;

#define DS_RECEIVE_PORT 1150
#define DS_SEND_PORT 1110

static int fd = -1;

static void dsSendLoop(uint64_t currentTime, HAL_NotifierHandle handle) {
  if (fd <= 0) goto done;

  struct sockaddr_in addr;
  memset((char *)&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_LOOPBACK;
  addr.sin_port = htons(DS_SEND_PORT);


  char buf[10];
  sendto(fd, buf, 10, 0, (struct sockaddr*) &addr, sizeof(addr));

done:
  int32_t status = 0;
  HAL_UpdateNotifierAlarm(handle, currentTime + 20000ULL, &status); // 20 milliseconds
}

bool StartDsLoop() {
  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    //llvm::errs << "Could not create socket";
    return false;
  }

  struct sockaddr_in addr;
  memset((char *)&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(0);

  if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    //llvm::errs << "bind failed";
    close(fd);
    fd = -1;
    return false;
  }

  int32_t status = 0;
  sendThreadHandle = HAL_InitializeNotifierThreaded(dsSendLoop, nullptr, &status);
  if (status != 0) {
    close(fd);
    fd = -1;
    return false;
  }
  uint64_t curTime = HAL_GetFPGATime(&status);
  if (status == 0)
    HAL_UpdateNotifierAlarm(sendThreadHandle, curTime + 20000ULL,
                            &status);
  return true;
}

void SetJoystickHookedUp(int joystick, bool isHookedUp) {

}

void SetJoystickButtonCount(int joystick, int count) {

}

void SetJoystickAxisCount(int joystick, int count) {

}

void SetJoystickPOVCount(int joystick, int count) {

}

void PutJoystickAxis(int joystick, int axis, float val) {

}

void PutJoystickPOV(int joystick, int povNum, int val) {

}

void PutJoystickButton(int joystick, int button, bool value) {

}

void PutJoystickButtons(int joystick, uint32_t value) {

}

void PutJoystickName(int joystick, const char* name) {
  std::lock_guard<priority_mutex> lock(joystickMutex);
  joysticks[joystick].name = llvm::StringRef(name, std::min(255, (int)strlen(name)));
}

uint32_t GetJoystickOutputs(int joystick) {
  std::lock_guard<priority_mutex> lock(joystickMutex);
  return joysticks[joystick].outputs;
}

void GetJoystickRumble(int joystick, int* left, int* right) {

}
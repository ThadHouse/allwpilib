// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

#include <networktables/BooleanTopic.h>
#include <networktables/NetworkTableInstance.h>
#include <networktables/ProtobufTopic.h>
#include <networktables/StringArrayTopic.h>
#include <networktables/StringTopic.h>

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <string>
#include <string_view>
#include <utility>

#include <fmt/format.h>
#include <wpi/EventVector.h>
#include <wpi/SafeThread.h>
#include <wpi/SmallVector.h>
#include <wpi/condition_variable.h>
#include <wpi/mutex.h>
#include <wpi/timestamp.h>

#include "HALInitializer.h"
#include "SystemServer.h"
#include "hal/DriverStation.h"
#include "hal/Errors.h"
#include "hal/proto/ControlData.h"
#include "hal/proto/JoystickDescriptor.h"
#include "hal/proto/JoystickOutputData.h"
#include "hal/proto/MatchInfo.h"
#include "mrc/NtNetComm.h"

static_assert(sizeof(int32_t) >= sizeof(int),
              "FRC_NetworkComm status variable is larger than 32 bits");

static_assert(MRC_MAX_NUM_AXES == HAL_kMaxJoystickAxes);
static_assert(MRC_MAX_NUM_POVS == HAL_kMaxJoystickPOVs);
static_assert(MRC_MAX_NUM_JOYSTICKS == HAL_kMaxJoysticks);

namespace {
struct JoystickDataCache {
  JoystickDataCache() { std::memset(this, 0, sizeof(*this)); }
  void Update(const mrc::ControlData& data);

  HAL_JoystickAxes axes[HAL_kMaxJoysticks];
  HAL_JoystickPOVs povs[HAL_kMaxJoysticks];
  HAL_JoystickButtons buttons[HAL_kMaxJoysticks];
  HAL_AllianceStationID allianceStation;
  float matchTime;
  HAL_ControlWord controlWord;
};
static_assert(std::is_standard_layout_v<JoystickDataCache>);
// static_assert(std::is_trivial_v<JoystickDataCache>);

struct SystemServerDriverStation {
  nt::NetworkTableInstance ntInst;
  nt::BooleanPublisher robotProgramPublisher;
  nt::BooleanPublisher codeStartedPublisher;
  nt::BooleanPublisher userCodeDisabledPublisher;
  nt::BooleanPublisher userCodeAutonomousPublisher;
  nt::BooleanPublisher userCodeTeleopPublisher;
  nt::BooleanPublisher userCodeTestPublisher;

  nt::ProtobufSubscriber<mrc::ControlData> controlDataSubscriber;
  nt::ProtobufSubscriber<mrc::MatchInfo> matchInfoSubscriber;
  nt::StringSubscriber gameSpecificMessageSubscriber;

  std::array<nt::ProtobufSubscriber<mrc::JoystickDescriptor>,
             MRC_MAX_NUM_JOYSTICKS>
      joystickDescriptorTopics;

  nt::StringPublisher versionPublisher;
  nt::StringPublisher consoleLinePublisher;

  std::array<nt::ProtobufPublisher<mrc::JoystickOutputData>,
             MRC_MAX_NUM_JOYSTICKS>
      joystickOutputsTopics;

  explicit SystemServerDriverStation(nt::NetworkTableInstance inst) {
    ntInst = inst;

    nt::PubSubOptions options;
    options.sendAll = true;
    options.keepDuplicates = true;
    options.periodic = 0.005;

    codeStartedPublisher =
        ntInst.GetBooleanTopic(ROBOT_CODE_STARTED_PATH).Publish(options);
    userCodeDisabledPublisher =
        ntInst.GetBooleanTopic(ROBOT_DISABLED_TRACE_PATH).Publish(options);
    userCodeAutonomousPublisher =
        ntInst.GetBooleanTopic(ROBOT_AUTON_TRACE_PATH).Publish(options);
    userCodeTeleopPublisher =
        ntInst.GetBooleanTopic(ROBOT_TELEOP_TRACE_PATH).Publish(options);
    userCodeTestPublisher =
        ntInst.GetBooleanTopic(ROBOT_TEST_TRACE_PATH).Publish(options);

    for (size_t count = 0; count < joystickOutputsTopics.size(); count++) {
      std::string name = ROBOT_JOYSTICK_OUTPUTS_PATH;
      name += std::to_string(count);
      joystickOutputsTopics[count] =
          ntInst.GetProtobufTopic<mrc::JoystickOutputData>(name).Publish(
              options);
    }

    robotProgramPublisher =
        ntInst.GetBooleanTopic(ROBOT_NEW_ROBOT_PROGRAM_PATH).Publish();
    robotProgramPublisher.Set(true);

    consoleLinePublisher =
        ntInst.GetStringTopic(ROBOT_CONSOLE_LINE_PATH).Publish(options);

    versionPublisher =
        ntInst.GetStringTopic(ROBOT_LIB_VERSION_PATH).Publish(options);

    controlDataSubscriber =
        ntInst.GetProtobufTopic<mrc::ControlData>(ROBOT_CONTROL_DATA_PATH)
            .Subscribe({}, options);

    matchInfoSubscriber =
        ntInst.GetProtobufTopic<mrc::MatchInfo>(ROBOT_MATCH_INFO_PATH)
            .Subscribe({});
    gameSpecificMessageSubscriber =
        ntInst.GetStringTopic(ROBOT_GAME_SPECIFIC_MESSAGE_PATH).Subscribe({});

    for (size_t count = 0; count < joystickDescriptorTopics.size(); count++) {
      std::string name = ROBOT_JOYSTICK_DESCRIPTORS_PATH;
      name += std::to_string(count);
      joystickDescriptorTopics[count] =
          ntInst.GetProtobufTopic<mrc::JoystickDescriptor>(name).Subscribe({});
    }
  }
};

struct FRCDriverStation {
  wpi::EventVector newDataEvents;
};
}  // namespace

static ::SystemServerDriverStation* systemServerDs;
static ::FRCDriverStation* driverStation;

// Message and Data variables
static wpi::mutex msgMutex;

void JoystickDataCache::Update(const mrc::ControlData& data) {
  matchTime = data.MatchTime;
  uint32_t allianceInt = data.ControlWord.Alliance;
  allianceInt += 1;
  allianceStation = static_cast<HAL_AllianceStationID>(allianceInt);

  std::memset(&controlWord, 0, sizeof(controlWord));
  controlWord.enabled = data.ControlWord.Enabled;
  controlWord.fmsAttached = data.ControlWord.FmsConnected;
  controlWord.dsAttached = data.ControlWord.DsConnected;
  controlWord.eStop = data.ControlWord.EStop;

  auto mode = data.GetOpMode();
  if (mode == "Test") {
    controlWord.test = true;
  } else if (mode == "Auton") {
    controlWord.autonomous = true;
  }

  auto sticks = data.Joysticks();

  for (size_t count = 0; count < sticks.size(); count++) {
    auto& newStick = sticks[count];
    auto newAxes = newStick.Axes.Axes();
    auto newPovs = newStick.Povs.Povs();

    axes[count].count = newAxes.size();
    for (size_t i = 0; i < newAxes.size(); i++) {
      axes[count].axes[i] = newAxes[i];
    }

    povs[count].count = newPovs.size();
    for (size_t i = 0; i < newPovs.size(); i++) {
      povs[count].povs[i] = newPovs[i];
    }

    buttons->count = newStick.Buttons.GetCount();
    buttons->buttons = newStick.Buttons.Buttons;
  }
}

#define CHECK_JOYSTICK_NUMBER(stickNum)                  \
  if ((stickNum) < 0 || (stickNum) >= HAL_kMaxJoysticks) \
  return PARAMETER_OUT_OF_RANGE

static HAL_ControlWord newestControlWord;
static JoystickDataCache caches[2];
static JoystickDataCache* currentRead = &caches[0];
static JoystickDataCache* cacheToUpdate = &caches[1];

static wpi::mutex cacheMutex;

namespace {
struct TcpCache {
  TcpCache() { std::memset(this, 0, sizeof(*this)); }
  void Update();
  void CloneTo(TcpCache* other) { std::memcpy(other, this, sizeof(*this)); }

  HAL_MatchInfo matchInfo;
  HAL_JoystickDescriptor descriptors[HAL_kMaxJoysticks];
};
static_assert(std::is_standard_layout_v<TcpCache>);
}  // namespace

static TcpCache tcpCaches[2];
static TcpCache* tcpCurrentRead = &tcpCaches[0];
static TcpCache* tcpCacheToUpdate = &tcpCaches[1];

static wpi::mutex tcpCacheMutex;

void TcpCache::Update() {
  auto newMatchInfo = systemServerDs->matchInfoSubscriber.Get();
  auto gameMsg = systemServerDs->gameSpecificMessageSubscriber.Get();

  matchInfo.matchNumber = newMatchInfo.MatchNumber;
  matchInfo.matchType = static_cast<HAL_MatchType>(newMatchInfo.Type);
  matchInfo.replayNumber = newMatchInfo.ReplayNumber;

  auto newEventName = newMatchInfo.GetEventName();
  auto nameLen =
      (std::min)(sizeof(matchInfo.eventName) - 1, newEventName.size());

  if (nameLen > 0) {
    std::memcpy(matchInfo.eventName, newEventName.data(), nameLen);
  }
  matchInfo.eventName[nameLen] = '\0';

  auto gameDataLen =
      (std::min)(sizeof(matchInfo.gameSpecificMessage), gameMsg.size());

  if (gameDataLen > 0) {
    std::memcpy(matchInfo.gameSpecificMessage, gameMsg.data(), gameDataLen);
  }
  matchInfo.gameSpecificMessageSize = gameDataLen;

  for (size_t count = 0;
       count < systemServerDs->joystickDescriptorTopics.size(); count++) {
    auto newDesc = systemServerDs->joystickDescriptorTopics[count].Get();

    auto& desc = descriptors[count];

    desc.isXbox = newDesc.IsXbox;
    desc.type = newDesc.Type;
    desc.buttonCount = newDesc.GetButtonsCount();
    desc.povCount = newDesc.GetPovsCount();

    auto joystickName = newDesc.GetName();
    auto joystickNameLen =
        (std::min)(sizeof(desc.name) - 1, joystickName.size());

    if (joystickNameLen > 0) {
      std::memcpy(desc.name, joystickName.data(), joystickNameLen);
    }
    desc.name[joystickNameLen] = '\0';

    auto sticks = newDesc.AxesTypes();

    desc.axisCount = sticks.size();

    for (size_t i = 0; i < sticks.size(); i++) {
      desc.axisTypes[i] = sticks[i];
    }
  }
}

namespace hal::init {
void InitializeFRCDriverStation() {
  std::memset(&newestControlWord, 0, sizeof(newestControlWord));
  static FRCDriverStation ds;
  driverStation = &ds;
}
}  // namespace hal::init

namespace hal {
static void DefaultPrintErrorImpl(const char* line, size_t size) {
  std::fwrite(line, size, 1, stderr);
}
}  // namespace hal

static std::atomic<void (*)(const char* line, size_t size)> gPrintErrorImpl{
    hal::DefaultPrintErrorImpl};

extern "C" {

int32_t HAL_SendError(HAL_Bool isError, int32_t errorCode, HAL_Bool isLVCode,
                      const char* details, const char* location,
                      const char* callStack, HAL_Bool printMsg) {
  return 0;
  // // Avoid flooding console by keeping track of previous 5 error
  // // messages and only printing again if they're longer than 1 second old.
  // static constexpr int KEEP_MSGS = 5;
  // std::scoped_lock lock(msgMutex);
  // static std::string prevMsg[KEEP_MSGS];
  // static std::chrono::time_point<std::chrono::steady_clock>
  //     prevMsgTime[KEEP_MSGS];
  // static bool initialized = false;
  // if (!initialized) {
  //   for (int i = 0; i < KEEP_MSGS; i++) {
  //     prevMsgTime[i] =
  //         std::chrono::steady_clock::now() - std::chrono::seconds(2);
  //   }
  //   initialized = true;
  // }

  // auto curTime = std::chrono::steady_clock::now();
  // int i;
  // for (i = 0; i < KEEP_MSGS; ++i) {
  //   if (prevMsg[i] == details) {
  //     break;
  //   }
  // }
  // int retval = 0;
  // if (i == KEEP_MSGS || (curTime - prevMsgTime[i]) >=
  // std::chrono::seconds(1)) {
  //   std::string_view detailsRef{details};
  //   std::string_view locationRef{location};
  //   std::string_view callStackRef{callStack};

  //   // 2 size, 1 tag, 4 timestamp, 2 seqnum
  //   // 2 numOccur, 4 error code, 1 flags, 6 strlen
  //   // 1 extra needed for padding on Netcomm end.
  //   size_t baseLength = 23;

  //   if (baseLength + detailsRef.size() + locationRef.size() +
  //           callStackRef.size() <=
  //       65535) {
  //     // Pass through
  //     retval = FRC_NetworkCommunication_sendError(isError, errorCode,
  //     isLVCode,
  //                                                 details, location,
  //                                                 callStack);
  //   } else if (baseLength + detailsRef.size() > 65535) {
  //     // Details too long, cut both location and stack
  //     auto newLen = 65535 - baseLength;
  //     std::string newDetails{details, newLen};
  //     char empty = '\0';
  //     retval = FRC_NetworkCommunication_sendError(
  //         isError, errorCode, isLVCode, newDetails.c_str(), &empty, &empty);
  //   } else if (baseLength + detailsRef.size() + locationRef.size() > 65535) {
  //     // Location too long, cut stack
  //     auto newLen = 65535 - baseLength - detailsRef.size();
  //     std::string newLocation{location, newLen};
  //     char empty = '\0';
  //     retval = FRC_NetworkCommunication_sendError(
  //         isError, errorCode, isLVCode, details, newLocation.c_str(),
  //         &empty);
  //   } else {
  //     // Stack too long
  //     auto newLen = 65535 - baseLength - detailsRef.size() -
  //     locationRef.size(); std::string newCallStack{callStack, newLen}; retval
  //     = FRC_NetworkCommunication_sendError(isError, errorCode, isLVCode,
  //                                                 details, location,
  //                                                 newCallStack.c_str());
  //   }
  //   if (printMsg) {
  //     fmt::memory_buffer buf;
  //     if (location && location[0] != '\0') {
  //       fmt::format_to(fmt::appender{buf},
  //                      "{} at {}: ", isError ? "Error" : "Warning",
  //                      location);
  //     }
  //     fmt::format_to(fmt::appender{buf}, "{}\n", details);
  //     if (callStack && callStack[0] != '\0') {
  //       fmt::format_to(fmt::appender{buf}, "{}\n", callStack);
  //     }
  //     auto printError = gPrintErrorImpl.load();
  //     printError(buf.data(), buf.size());
  //   }
  //   if (i == KEEP_MSGS) {
  //     // replace the oldest one
  //     i = 0;
  //     auto first = prevMsgTime[0];
  //     for (int j = 1; j < KEEP_MSGS; ++j) {
  //       if (prevMsgTime[j] < first) {
  //         first = prevMsgTime[j];
  //         i = j;
  //       }
  //     }
  //     prevMsg[i] = details;
  //   }
  //   prevMsgTime[i] = curTime;
  // }
  // return retval;
}

void HAL_SetPrintErrorImpl(void (*func)(const char* line, size_t size)) {
  gPrintErrorImpl.store(func ? func : hal::DefaultPrintErrorImpl);
}

int32_t HAL_SendConsoleLine(const char* line) {
  systemServerDs->consoleLinePublisher.Set(line);
  return 0;
}

int32_t HAL_GetControlWord(HAL_ControlWord* controlWord) {
  std::scoped_lock lock{cacheMutex};
  *controlWord = newestControlWord;
  return 0;
}

int32_t HAL_GetJoystickAxes(int32_t joystickNum, HAL_JoystickAxes* axes) {
  CHECK_JOYSTICK_NUMBER(joystickNum);
  std::scoped_lock lock{cacheMutex};
  *axes = currentRead->axes[joystickNum];
  return 0;
}

int32_t HAL_GetJoystickPOVs(int32_t joystickNum, HAL_JoystickPOVs* povs) {
  CHECK_JOYSTICK_NUMBER(joystickNum);
  std::scoped_lock lock{cacheMutex};
  *povs = currentRead->povs[joystickNum];
  return 0;
}

int32_t HAL_GetJoystickButtons(int32_t joystickNum,
                               HAL_JoystickButtons* buttons) {
  CHECK_JOYSTICK_NUMBER(joystickNum);
  std::scoped_lock lock{cacheMutex};
  *buttons = currentRead->buttons[joystickNum];
  return 0;
}

void HAL_GetAllJoystickData(HAL_JoystickAxes* axes, HAL_JoystickPOVs* povs,
                            HAL_JoystickButtons* buttons) {
  std::scoped_lock lock{cacheMutex};
  std::memcpy(axes, currentRead->axes, sizeof(currentRead->axes));
  std::memcpy(povs, currentRead->povs, sizeof(currentRead->povs));
  std::memcpy(buttons, currentRead->buttons, sizeof(currentRead->buttons));
}

int32_t HAL_GetJoystickDescriptor(int32_t joystickNum,
                                  HAL_JoystickDescriptor* desc) {
  CHECK_JOYSTICK_NUMBER(joystickNum);
  std::scoped_lock lock{tcpCacheMutex};
  *desc = tcpCurrentRead->descriptors[joystickNum];
  return 0;
}

int32_t HAL_GetMatchInfo(HAL_MatchInfo* info) {
  std::scoped_lock lock{tcpCacheMutex};
  *info = tcpCurrentRead->matchInfo;
  return 0;
}

HAL_AllianceStationID HAL_GetAllianceStation(int32_t* status) {
  std::scoped_lock lock{cacheMutex};
  return currentRead->allianceStation;
}

HAL_Bool HAL_GetJoystickIsXbox(int32_t joystickNum) {
  HAL_JoystickDescriptor joystickDesc;
  if (HAL_GetJoystickDescriptor(joystickNum, &joystickDesc) < 0) {
    return 0;
  } else {
    return joystickDesc.isXbox;
  }
}

int32_t HAL_GetJoystickType(int32_t joystickNum) {
  HAL_JoystickDescriptor joystickDesc;
  if (HAL_GetJoystickDescriptor(joystickNum, &joystickDesc) < 0) {
    return -1;
  } else {
    return joystickDesc.type;
  }
}

void HAL_GetJoystickName(struct WPI_String* name, int32_t joystickNum) {
  HAL_JoystickDescriptor joystickDesc;
  const char* cName = joystickDesc.name;
  if (HAL_GetJoystickDescriptor(joystickNum, &joystickDesc) < 0) {
    cName = "";
  }
  auto len = std::strlen(cName);
  auto write = WPI_AllocateString(name, len);
  std::memcpy(write, cName, len);
}

int32_t HAL_GetJoystickAxisType(int32_t joystickNum, int32_t axis) {
  HAL_JoystickDescriptor joystickDesc;
  if (HAL_GetJoystickDescriptor(joystickNum, &joystickDesc) < 0) {
    return -1;
  } else {
    return joystickDesc.axisTypes[axis];
  }
}

int32_t HAL_SetJoystickOutputs(int32_t joystickNum, int64_t outputs,
                               int32_t leftRumble, int32_t rightRumble) {
  CHECK_JOYSTICK_NUMBER(joystickNum);

  mrc::JoystickOutputData outputData{
      .HidOutputs = static_cast<uint32_t>(outputs),
      .LeftRumble = std::clamp(leftRumble, 0, UINT16_MAX) /
                    static_cast<float>(UINT16_MAX),
      .RightRumble = std::clamp(rightRumble, 0, UINT16_MAX) /
                     static_cast<float>(UINT16_MAX),
  };

  systemServerDs->joystickOutputsTopics[joystickNum].Set(outputData);

  return 0;
}

double HAL_GetMatchTime(int32_t* status) {
  std::scoped_lock lock{cacheMutex};
  return currentRead->matchTime;
}

void HAL_ObserveUserProgramStarting(void) {
  systemServerDs->codeStartedPublisher.Set(true);
}

void HAL_ObserveUserProgramDisabled(void) {
  systemServerDs->userCodeDisabledPublisher.Set(true);
}

void HAL_ObserveUserProgramAutonomous(void) {
  systemServerDs->userCodeAutonomousPublisher.Set(true);
}

void HAL_ObserveUserProgramTeleop(void) {
  systemServerDs->userCodeTeleopPublisher.Set(true);
}

void HAL_ObserveUserProgramTest(void) {
  systemServerDs->userCodeTestPublisher.Set(true);
}

HAL_Bool HAL_RefreshDSData(void) {
  auto newestData = systemServerDs->controlDataSubscriber.GetAtomic({});

  auto now = wpi::Now();
  auto delta = now - newestData.time;

  bool updatedData = false;

  // Data newer then 125ms, and we have a DS connected
  if (delta < 125000 && newestData.value.ControlWord.DsConnected) {
    // Update the cache.
    cacheToUpdate->Update(newestData.value);
    updatedData = true;
  } else {
    // DS disconnected. Clear the control word
    std::memset(&cacheToUpdate->controlWord, 0,
                sizeof(cacheToUpdate->controlWord));
  }

  {
    std::scoped_lock lock{cacheMutex};
    std::swap(cacheToUpdate, currentRead);
    newestControlWord = currentRead->controlWord;
  }

  tcpCacheToUpdate->Update();
  std::scoped_lock tcpLock(tcpCacheMutex);
  std::swap(tcpCacheToUpdate, tcpCurrentRead);
  return updatedData;
}

void HAL_ProvideNewDataEventHandle(WPI_EventHandle handle) {
  hal::init::CheckInit();
  driverStation->newDataEvents.Add(handle);
}

void HAL_RemoveNewDataEventHandle(WPI_EventHandle handle) {
  driverStation->newDataEvents.Remove(handle);
}

HAL_Bool HAL_GetOutputsEnabled(void) {
  return systemServerDs->controlDataSubscriber.Get().ControlWord.WatchdogActive;
}

}  // extern "C"

namespace hal {
void InitializeDriverStation() {
  systemServerDs = new ::SystemServerDriverStation{hal::GetSystemServer()};
}

void WaitForInitialPacket() {
  wpi::Event waitForInitEvent;
  driverStation->newDataEvents.Add(waitForInitEvent.GetHandle());
  bool timed_out = false;
  wpi::WaitForObject(waitForInitEvent.GetHandle(), 0.1, &timed_out);
  // Don't care what the result is, just want to give it a chance.
  driverStation->newDataEvents.Remove(waitForInitEvent.GetHandle());
}
}  // namespace hal
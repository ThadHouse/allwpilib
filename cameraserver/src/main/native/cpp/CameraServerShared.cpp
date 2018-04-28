/*----------------------------------------------------------------------------*/
/* Copyright (c) 2018 FIRST. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "CameraServerShared.h"

#include <support/mutex.h>

namespace frc {
static CameraServerShared cameraServerShared;
static bool cameraServerSet = false;
static wpi::mutex setLock;

void SetCameraServerShared(const CameraServerShared& shared) {
  std::unique_lock<wpi::mutex> lock(setLock);
  cameraServerSet = true;
  cameraServerShared = shared;
}
const CameraServerShared& GetCameraServerShared() {
  std::unique_lock<wpi::mutex> lock(setLock);
  if (!cameraServerSet) {
    cameraServerSet = true;
    cameraServerShared.GetRobotMainThreadId = []() {
      return std::make_pair(std::thread::id(), false);
    };
    cameraServerShared.ReportAxisCamera = [](int id) {};
    cameraServerShared.ReportUsbCamera = [](int id) {};
    cameraServerShared.ReportVideoServer = [](int id) {};
    cameraServerShared.SetCameraServerError = [](llvm::StringRef err) {};
    cameraServerShared.SetVisionRunnerError = [](llvm::StringRef err) {};
    cameraServerShared.ReportDriverStationError = [](llvm::StringRef err) {};
  }
  return cameraServerShared;
}
}  // namespace frc

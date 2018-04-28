/*----------------------------------------------------------------------------*/
/* Copyright (c) 2018 FIRST. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#pragma once

#include <thread>
#include <utility>

#include <llvm/StringRef.h>

namespace frc {

typedef void (*ReportUsbCameraFunc)(int);
typedef void (*ReportAxisCameraFunc)(int);
typedef void (*ReportVideoServerFunc)(int);
typedef void (*SetCameraServerErrorFunc)(llvm::StringRef);
typedef void (*SetVisionRunnerErrorFunc)(llvm::StringRef);
typedef void (*ReportDriverStationErrorFunc)(llvm::StringRef);
typedef std::pair<std::thread::id, bool> (*GetRobotMainThreadIdFunc)();

struct CameraServerShared {
  ReportUsbCameraFunc ReportUsbCamera;
  ReportAxisCameraFunc ReportAxisCamera;
  ReportVideoServerFunc ReportVideoServer;
  SetCameraServerErrorFunc SetCameraServerError;
  SetVisionRunnerErrorFunc SetVisionRunnerError;
  ReportDriverStationErrorFunc ReportDriverStationError;
  GetRobotMainThreadIdFunc GetRobotMainThreadId;
};

void SetCameraServerShared(const CameraServerShared& shared);
const CameraServerShared& GetCameraServerShared();
}  // namespace frc

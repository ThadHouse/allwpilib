/*----------------------------------------------------------------------------*/
/* Copyright (c) 2019 FIRST. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include <hal/Main.h>
#include <wpi/raw_ostream.h>

#include "AccelerometerGui.h"
#include "AnalogGyroGui.h"
#include "AnalogInputGui.h"
#include "AnalogOutGui.h"
#include "CompressorGui.h"
#include "DriverStationGui.h"
#include "HALSimGui.h"
#include "PWMGui.h"
#include "SimDeviceGui.h"
#include "SolenoidGui.h"

using namespace halsimgui;

extern "C" {
#if defined(WIN32) || defined(_WIN32)
__declspec(dllexport)
#endif
    int HALSIM_InitExtension(void) {
  HALSimGui::Add(AccelerometerGui::Initialize, AccelerometerGui::Execute);
  HALSimGui::Add(AnalogGyroGui::Initialize, AnalogGyroGui::Execute);
  HALSimGui::Add(AnalogInputGui::Initialize, AnalogInputGui::Execute);
  HALSimGui::Add(AnalogOutGui::Initialize, AnalogOutGui::Execute);
  HALSimGui::Add(CompressorGui::Initialize, CompressorGui::Execute);
  HALSimGui::Add(DriverStationGui::Initialize, DriverStationGui::Execute);
  HALSimGui::Add(PWMGui::Initialize, PWMGui::Execute);
  HALSimGui::Add(SimDeviceGui::Initialize, SimDeviceGui::Execute);
  HALSimGui::Add(SolenoidGui::Initialize, SolenoidGui::Execute);

  wpi::outs() << "Simulator GUI Initializing.\n";
  if (!HALSimGui::Initialize()) return 0;
  HAL_SetMain(nullptr, HALSimGui::Main, HALSimGui::Exit);
  wpi::outs() << "Simulator GUI Initialized!\n";

  return 0;
}
}  // extern "C"

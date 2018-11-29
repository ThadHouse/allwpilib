/*----------------------------------------------------------------------------*/
/* Copyright (c) 2018 FIRST. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#pragma once

#include "frc/smartdashboard/SendableBase.h"
#include "cscore_oo.h"
#include <string>

namespace frc {
class SendableCameraWrapper final : public SendableBase {
 public:
  static SendableCameraWrapper Wrap(cs::VideoSource source);
  void InitSendable(SendableBuilder& builder) override;
 private:
  SendableCameraWrapper(cs::VideoSource source);

  std::string m_uri;
};
} // namespace frc

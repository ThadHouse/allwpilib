// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

#include "frc/kinematics/proto/DifferentialDriveWheelSpeedsProto.h"

#include "kinematics.npb.h"

const pb_msgdesc_t*
wpi::Protobuf<frc::DifferentialDriveWheelSpeeds>::Message() {
  return get_wpi_proto_ProtobufDifferentialDriveWheelSpeeds_msg();
}

std::optional<frc::DifferentialDriveWheelSpeeds> wpi::Protobuf<
    frc::DifferentialDriveWheelSpeeds>::Unpack(wpi::ProtoInputStream& stream) {
  wpi_proto_ProtobufDifferentialDriveWheelSpeeds msg;
  if (!stream.DecodeNoInit(msg)) {
    return {};
  }

  return frc::DifferentialDriveWheelSpeeds{
      units::meters_per_second_t{msg.left},
      units::meters_per_second_t{msg.right},
  };
}

bool wpi::Protobuf<frc::DifferentialDriveWheelSpeeds>::Pack(
    wpi::ProtoOutputStream& stream,
    const frc::DifferentialDriveWheelSpeeds& value) {
  wpi_proto_ProtobufDifferentialDriveWheelSpeeds msg{
      .left = value.left.value(),
      .right = value.right.value(),
  };
  return stream.Encode(msg);
}

// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

#include "frc/kinematics/proto/SwerveModulePositionProto.h"

#include "kinematics.npb.h"
#include "wpi/protobuf/ProtobufCallbacks.h"

const pb_msgdesc_t* wpi::Protobuf<frc::SwerveModulePosition>::Message() {
  return get_wpi_proto_ProtobufSwerveModulePosition_msg();
}

std::optional<frc::SwerveModulePosition> wpi::Protobuf<
    frc::SwerveModulePosition>::Unpack(wpi::ProtoInputStream& stream) {
  wpi::UnpackCallback<frc::Rotation2d> angle;
  wpi_proto_ProtobufSwerveModulePosition msg{
      .angle = angle.Callback(),
  };
  if (!stream.DecodeNoInit(msg)) {
    return {};
  }

  auto iangle = angle.Items();

  if (iangle.empty()) {
    return {};
  }

  return frc::SwerveModulePosition{
      units::meter_t{msg.distance},
      iangle[0],
  };
}

bool wpi::Protobuf<frc::SwerveModulePosition>::Pack(
    wpi::ProtoOutputStream& stream, const frc::SwerveModulePosition& value) {
  wpi::PackCallback angle{&value.angle};
  wpi_proto_ProtobufSwerveModulePosition msg{
      .distance = value.distance.value(),
      .angle = angle.Callback(),
  };
  return stream.Encode(msg);
}

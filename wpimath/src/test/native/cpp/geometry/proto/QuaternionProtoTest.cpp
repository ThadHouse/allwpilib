// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

#include <google/protobuf/arena.h>
#include <gtest/gtest.h>

#include "frc/geometry/Quaternion.h"

using namespace frc;

namespace {

using ProtoType = wpi::Protobuf<frc::Quaternion>;

const Quaternion kExpectedData = Quaternion{1.1, 0.191, 35.04, 19.1};
}  // namespace

TEST(QuaternionProtoTest, Roundtrip) {
  wpi::ProtobufMessage<Quaternion> message;
  wpi::SmallVector<uint8_t, 64> buf;

  ASSERT_TRUE(message.Pack(buf, kExpectedData));
  std::optional<Quaternion> unpacked_data = message.Unpack(buf);
  ASSERT_TRUE(unpacked_data.has_value());

  EXPECT_EQ(kExpectedData.W(), unpacked_data.W());
  EXPECT_EQ(kExpectedData.X(), unpacked_data.X());
  EXPECT_EQ(kExpectedData.Y(), unpacked_data.Y());
  EXPECT_EQ(kExpectedData.Z(), unpacked_data.Z());
}

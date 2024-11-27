// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

#include <frc/TimedRobot.h>

#include "wpinet/MulticastServiceResolver.h"

class Robot : public frc::TimedRobot {
 public:
  wpi::MulticastServiceResolver* Res;

  /**
   * This function is run when the robot is first started up and should be
   * used for any initialization code.
   */
  Robot() {
    Res = new wpi::MulticastServiceResolver{"_ni._tcp"};
    Res->Start();
  }

  /**
   * This function is run once each time the robot enters autonomous mode
   */
  void AutonomousInit() override {}

  /**
   * This function is called periodically during autonomous
   */
  void AutonomousPeriodic() override {}

  /**
   * This function is called once each time the robot enters tele-operated mode
   */
  void TeleopInit() override {}

  /**
   * This function is called periodically during operator control
   */
  void TeleopPeriodic() override {}

  /**
   * This function is called periodically during test mode
   */
  void TestPeriodic() override {}

  /**
   * This function is called periodically during all modes
   */
  void RobotPeriodic() override {
    auto items = Res->GetData();
    if (!items.empty()) {
      fmt::println("Stopping");
      std::fflush(stdout);
      Res->Stop();
      fmt::println("Stopped");
      std::fflush(stdout);
    }
  }
};

int main() {
  return frc::StartRobot<Robot>();
}

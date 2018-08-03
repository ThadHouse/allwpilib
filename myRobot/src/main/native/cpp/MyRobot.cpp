/*----------------------------------------------------------------------------*/
/* Copyright (c) 2016-2018 FIRST. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include <frc/DigitalInput.h>
#include <frc/IterativeRobot.h>
#include <frc/Solenoid.h>
#include <frc/smartdashboard/SmartDashboard.h>

class MyRobot : public frc::IterativeRobot {
  frc::Solenoid* solenoid;
  frc::DigitalInput* dio;
  int count = 0;
  /**
   * This function is run when the robot is first started up and should be
   * used for any initialization code.
   */
  void RobotInit() override {
    solenoid = new frc::Solenoid(4);
    dio = new frc::DigitalInput(9);

    frc::SmartDashboard::PutBoolean("Toggle", false);
    frc::SmartDashboard::PutBoolean("Conn", true);
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
    solenoid->Set(frc::SmartDashboard::GetBoolean("Toggle", false));
    frc::SmartDashboard::PutBoolean("Conn", dio->Get());
    frc::SmartDashboard::PutBoolean("SValue", solenoid->Get());
    frc::SmartDashboard::PutNumber("count", count);
    count++;
  }
};

int main() { return frc::StartRobot<MyRobot>(); }

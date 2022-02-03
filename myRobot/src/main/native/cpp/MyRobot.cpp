// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

#include <frc/TimedRobot.h>
#include <FRC_FPGA_ChipObject/fpgainterfacecapi/NiFpga_HMB.h>
#include <hal/ChipObject.h>
#include <frc/smartdashboard/SmartDashboard.h>
#include <hal/HMB.h>
#include <frc/motorcontrol/PWMSparkMax.h>
#include <frc/AnalogGyro.h>

class MyRobot : public frc::TimedRobot {
  frc::PWMSparkMax spark0{0};
  frc::PWMSparkMax spark9{9};
  frc::PWMSparkMax spark10{10};
  frc::PWMSparkMax spark19{19};
  frc::AnalogGyro gyro{1};

  //hal::tHMB* hmb;
  //uint32_t* buffer;
  /**
   * This function is run when the robot is first started up and should be
   * used for any initialization code.
   */
  void RobotInit() override {
    int32_t status = 0;
    HAL_InitializeHMB(&status);

    // aiHandle = HAL_InitializeAnalogInputPort(HAL_GetPort(0), nullptr, &status);

    // HAL_Init
    // hmb = hal::tHMB::create(&status);
    // printf("HMB Create %d\n", status);

    // printf("First read %d\n", hmb->readForceOnce(&status));

    // auto cfg = hmb->readConfig(&status);
    // cfg.Enables_Timestamp = 1;
    // //cfg.Enables_DI = 1;
    // //cfg.Enables_PWM = 1;
    // //cfg.Enables_AI0_Low = 1;
    // //cfg.Enables_AI0_High = 1;
    // hmb->writeConfig(cfg, &status);

    // uint32_t session = hmb->getSystemInterface()->getHandle();
    // size_t size = 0;
    // status = NiFpga_OpenHostMemoryBuffer(session, "HMB_0_RAM", (void**)&buffer,
    //                                      &size);
    // printf("status %d size %d\n", status, (int)size);
    // for (int i = 0; i < 2097152; i++) {
    //         buffer[i] = 0;
    //        }
    //        asm("dmb");
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
  void TeleopPeriodic() override {
    spark0.Set(-1);
    spark9.Set(-0.5);
    spark10.Set(0.5);
    spark19.Set(1);
    // //int32_t status = 0;
    // for (int i = 0; i < 1024; i++) {
    //   buffer[i] = 0;
    // }
    // hmb->writeForceOnce(true, &status);
    // //printf("status %d\n", status);
    // hmb->writeForceOnce(false, &status);
  }

  /**
   * This function is called periodically during test mode
   */
  void TestPeriodic() override {}

  /**
   * This function is called periodically during all modes
   */
  void RobotPeriodic() override {

    //asm("dmb");
    int32_t status = 0;

   // asm("dmb");
    //hmb->writeForceOnce(false, &status);
    // printf("%u %u %u %u %u %u %u %u\n", buffer[0], buffer[1], buffer[2],
    //        buffer[3], buffer[240], buffer[5], buffer[6], buffer[7]);

    auto buffer = HAL_GetHMBBuffer(0, &status);
    asm("dmb");

           for (int i = 0; i < 1024; i++) {
             if (buffer[i] != 0) {
               printf("FOUND %d %u\n", i, buffer[i]);
             }
           }

           //buffer[240] = 0xFF;
           //buffer[0] = 0xFF;
          // asm("dmb");

      // hmb->writeForceOnce(true, &status);
      // hmb->writeForceOnce(false, &status);

    HAL_StrobeHMB(0, &status);

    frc::SmartDashboard::PutString("DIO", fmt::format("{:#08x}", HAL_ReadHMBDIO(0, &status)));
    frc::SmartDashboard::PutString("Timestamp", fmt::format("{}", HAL_ReadHMBFPGATimestamp(0, &status)));

    // printf("readLoopCount %d\n", hmb->readLoopCount(&status));
    // printf("readWriteData %u\n", hmb->readWriteData(&status));
    // printf("readReadData %u\n", hmb->readReadData(&status));
    // printf("readWriteAddress %u\n", hmb->readWriteAddress(&status));

    // printf("readWriteCount %u\n", hmb->readWriteCount(&status));

    // printf("readReqReadyForInput %u\n", hmb->readReqReadyForInput(&status));

    // printf("readWriteReadyForInput %u\n", hmb->readWriteReadyForInput(&status));

    // auto cfg = hmb->readConfig(&status);
    // printf("cfg %d\n", cfg.Enables_Timestamp);
  }
};

int main() {
  return frc::StartRobot<MyRobot>();
}

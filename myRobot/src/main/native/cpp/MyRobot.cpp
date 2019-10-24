/*----------------------------------------------------------------------------*/
/* Copyright (c) 2016-2018 FIRST. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include <frc/TimedRobot.h>
#include "hal/DMA.h"
#include "hal/DIO.h"
#include "hal/AnalogInput.h"
#include "hal/HALBase.h"
#include "hal/DIO.h"
#include <iostream>
#include "units/units.h"
#include "frc/SPI.h"

using namespace units::literals;

HAL_DMAHandle dma;
HAL_DMASampleHandle sample;
HAL_AnalogInputHandle ai;
HAL_AnalogInputHandle a2;
HAL_AnalogInputHandle a3;
HAL_DigitalHandle dInput;
HAL_DigitalHandle dOutput;

frc::SPI* spi;

class MyRobot : public frc::TimedRobot {
public:
  MyRobot() : TimedRobot(500_ms) {
  }

  /**
   * This function is run when the robot is first started up and should be
   * used for any initialization code.
   */
  void RobotInit() override {
    spi = new frc::SPI{frc::SPI::Port::kOnboardCS0};
    spi->SetSampleDataOnLeadingEdge();
    spi->SetMSBFirst();
    spi->SetClockActiveHigh();
    spi->SetClockRate(250000);

    // std::cout << "Starting Robot Init" << std::endl;
    // int32_t status = 0;
    // dma = HAL_InitializeDMA(&status);
    // std::cout << "Init DMA" << status << std::endl;
    // sample = HAL_MakeDMASample(&status);
    // std::cout << "Make DMA Sample" << status << std::endl;

    // ai = HAL_InitializeAnalogInputPort(HAL_GetPort(0), &status);
    // a2 = HAL_InitializeAnalogInputPort(HAL_GetPort(2), &status);
    // a3 = HAL_InitializeAnalogInputPort(HAL_GetPort(3), &status);
    // std::cout << "Init AI" << status << std::endl;
    // dInput = HAL_InitializeDIOPort(HAL_GetPort(0), true, &status);
    // std::cout << "Init DI" << status << std::endl;
    // dOutput = HAL_InitializeDIOPort(HAL_GetPort(1), false, &status);
    // std::cout << "Init DO" << status << std::endl;

    // HAL_AddDMAAnalogInput(dma, ai, &status);
    // HAL_AddDMAAnalogInput(dma, a2, &status);
    // HAL_AddDMAAnalogInput(dma, a3, &status);
    // std::cout << "Add DMA AI" << status << std::endl;
    // HAL_SetDMAExternalTrigger(dma, dInput, HAL_AnalogTriggerType::HAL_Trigger_kFallingPulse, true, false, &status);
    // std::cout << "Set DMA Trigger" << status << std::endl;

    // HAL_SetDIO(dOutput, false, &status);
    // std::cout << "Set DIO False" << status << std::endl;

    // HAL_StartDMA(dma, 1024, &status);
    // std::cout << "StartDMA" << status << std::endl;


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
    uint8_t data[3];
    data[0] = 0x8; // REad Temp
    data[1] = 0;
    data[2] = 0;
    uint8_t dataOut[3];
    dataOut[0] = 0;
    dataOut[1] = 0;
    dataOut[2] = 0;
    spi->Transaction(data, dataOut, 2);
    std::cout << std::hex << (int)dataOut[0] << " " << (int)dataOut[1] << " " << (int)dataOut[2] << std::endl;
    // int32_t status = 0;
    // auto beforeSetDIO = HAL_GetFPGATime(&status);
    // HAL_SetDIO(dOutput, true, &status);
    
    // int32_t remaining = 0;
    
    // auto res = HAL_ReadDMA(dma, sample, 100, &remaining, &status);
    // std::cout << "Read DMA: " << res << " " << remaining << " " << status << std::endl;

    // double voltage = HAL_GetDMASampleAnalogInputVoltage(sample, ai, &status);
    // double v2 = HAL_GetDMASampleAnalogInputVoltage(sample, a2, &status);
    // double v3 = HAL_GetDMASampleAnalogInputVoltage(sample, a3, &status);
    // auto ts = HAL_GetDMASampleTime(sample, &status);
    // std::cout << "TS:" << ts  << " " << beforeSetDIO  << std::endl; 
    // std::cout << " Voltage: " << voltage << " " << v2 << " " << v3 << std::endl;

    // auto va1 = HAL_GetAnalogValue(ai, &status);
    // auto va2 = HAL_GetAnalogValue(a2, &status);
    // auto va3 = HAL_GetAnalogValue(a3, &status);
    // std::cout << " VDirect: " << va1 << " " << va2 << " " << va3 << std::endl;



    // HAL_SetDIO(dOutput, false, &status);
  }
};

int main() { return frc::StartRobot<MyRobot>(); }

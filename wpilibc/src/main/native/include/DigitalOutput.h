/*----------------------------------------------------------------------------*/
/* Copyright (c) 2008-2017 FIRST. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#pragma once

#include <memory>
#include <string>

#include <HAL/Types.h>

#include "DigitalSource.h"
#include "LiveWindow/LiveWindowSendable.h"
#include "networktables/NetworkTableEntry.h"

namespace frc {

class TriStateDigital

/**
 * Class to write to digital outputs.
 * Write values to the digital output channels. Other devices implemented
 * elsewhere will allocate channels automatically so for those devices it
 * shouldn't be done here.
 */
class DigitalOutput : public DigitalSource, public LiveWindowSendable {
 public:
  explicit DigitalOutput(int channel);
  virtual ~DigitalOutput();
  void Set(bool value);
  bool Get() const;
  int GetChannel() const override;
  void Pulse(double length);
  bool IsPulsing() const;
  void SetPWMRate(double rate);
  void EnablePWM(double initialDutyCycle);
  void DisablePWM();
  void UpdateDutyCycle(double dutyCycle);

  // Digital Source Interface
  HAL_Handle GetPortHandleForRouting() const override;
  AnalogTriggerType GetAnalogTriggerTypeForRouting() const override;
  bool IsAnalogTrigger() const override;

  void UpdateTable() override;
  void StartLiveWindowMode() override;
  void StopLiveWindowMode() override;
  std::string GetSmartDashboardType() const override;
  void InitTable(std::shared_ptr<nt::NetworkTable> subTable) override;

 private:
  explicit DigitalOutput(HAL_DigitalHandle handle);
  int m_channel;
  HAL_DigitalHandle m_handle;
  HAL_DigitalPWMHandle m_pwmGenerator;

  nt::NetworkTableEntry m_valueEntry;
  NT_EntryListener m_valueListener = 0;

  friend class TriStateDigital;
};

}  // namespace frc

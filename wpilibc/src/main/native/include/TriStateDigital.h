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

#include "DigitalInput.h"
#include "DigitalOutput.h"
#include "DigitalSource.h"
#include "LiveWindow/LiveWindowSendable.h"
#include "networktables/NetworkTableEntry.h"

namespace frc {

/**
 * Class to read or write from digital channels, with switching support.
 * A direction of true is input
 */
class TriStateDigital : public DigitalSource, public LiveWindowSendable {
 public:
  explicit TriStateDigital(int channel, bool input);
  virtual ~TriStateDigital();
  void Set(bool value);
  bool Get() const;
  void SetDirection(bool direction);
  bool GetDirection() const;
  int GetChannel() const override;

  // Ways to get a DigitalInput and a DigitalOutput
  DigitalInput MoveToDigitalInput();
  DigitalOutput MoveToDigitalOutput();

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
  explicit TriStateDigital(HAL_DigitalHandle handle, bool input);

  int m_channel;
  HAL_DigitalHandle m_handle;
  HAL_DigitalPWMHandle m_pwmGenerator;

  nt::NetworkTableEntry m_valueEntry;
  NT_EntryListener m_valueListener = 0;

  friend class DigitalInput;
  friend class DigitalOutput;
};

}  // namespace frc

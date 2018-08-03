/*----------------------------------------------------------------------------*/
/* Copyright (c) 2018 FIRST. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#pragma once

#ifndef __FRC_ROBORIO__

#include <memory>
#include <utility>

#include "CallbackStore.h"
#include "mockdata/PCMData.h"

namespace frc {
namespace sim {
class PCMSim {
 public:
  explicit PCMSim(int index) { m_index = index; }

  std::unique_ptr<CallbackStore> RegisterInitializedCallback(
      NotifyCallback callback, bool initialNotify) {
    auto store = std::make_unique<CallbackStore>(
        m_index, -1, callback,
        &HALSIM_CancelPCMInitializedCallback);
    store->SetUid(HALSIM_RegisterPCMInitializedCallback(
        m_index, &CallbackStoreThunk, store.get(), initialNotify));
    return store;
  }
  bool GetInitialized() {
    return HALSIM_GetPCMInitialized(m_index);
  }
  void SetInitialized( bool initialized) {
    HALSIM_SetPCMInitialized(m_index, initialized);
  }

  std::unique_ptr<CallbackStore> RegisterSolenoidOutputCallback(
      int channel, NotifyCallback callback, bool initialNotify) {
    auto store = std::make_unique<CallbackStore>(
        m_index, channel, -1, callback,
        &HALSIM_CancelPCMSolenoidOutputCallback);
    store->SetUid(HALSIM_RegisterPCMSolenoidOutputCallback(
        m_index, channel, &CallbackStoreThunk, store.get(), initialNotify));
    return store;
  }
  bool GetSolenoidOutput(int channel) {
    return HALSIM_GetPCMSolenoidOutput(m_index, channel);
  }
  void SetSolenoidOutput(int channel, bool solenoidOutput) {
    HALSIM_SetPCMSolenoidOutput(m_index, channel, solenoidOutput);
  }

  int32_t GetAllSolenoidOutputs() {
    return HALSIM_GetPCMAllSolenoidOutputs(m_index);
  }
  void SetAllSolenoidOutputs(int32_t solenoidOutputs) {
    HALSIM_SetPCMAllSolenoidOutputs(m_index, solenoidOutputs);
  }


  std::unique_ptr<CallbackStore> RegisterCompressorOnCallback(
      NotifyCallback callback, bool initialNotify) {
    auto store = std::make_unique<CallbackStore>(
        m_index, -1, callback, &HALSIM_CancelPCMCompressorOnCallback);
    store->SetUid(HALSIM_RegisterPCMCompressorOnCallback(
        m_index, &CallbackStoreThunk, store.get(), initialNotify));
    return store;
  }
  bool GetCompressorOn() { return HALSIM_GetPCMCompressorOn(m_index); }
  void SetCompressorOn(bool compressorOn) {
    HALSIM_SetPCMCompressorOn(m_index, compressorOn);
  }

  std::unique_ptr<CallbackStore> RegisterClosedLoopEnabledCallback(
      NotifyCallback callback, bool initialNotify) {
    auto store = std::make_unique<CallbackStore>(
        m_index, -1, callback, &HALSIM_CancelPCMClosedLoopEnabledCallback);
    store->SetUid(HALSIM_RegisterPCMClosedLoopEnabledCallback(
        m_index, &CallbackStoreThunk, store.get(), initialNotify));
    return store;
  }
  bool GetClosedLoopEnabled() {
    return HALSIM_GetPCMClosedLoopEnabled(m_index);
  }
  void SetClosedLoopEnabled(bool closedLoopEnabled) {
    HALSIM_SetPCMClosedLoopEnabled(m_index, closedLoopEnabled);
  }

  std::unique_ptr<CallbackStore> RegisterPressureSwitchCallback(
      NotifyCallback callback, bool initialNotify) {
    auto store = std::make_unique<CallbackStore>(
        m_index, -1, callback, &HALSIM_CancelPCMPressureSwitchCallback);
    store->SetUid(HALSIM_RegisterPCMPressureSwitchCallback(
        m_index, &CallbackStoreThunk, store.get(), initialNotify));
    return store;
  }
  bool GetPressureSwitch() { return HALSIM_GetPCMPressureSwitch(m_index); }
  void SetPressureSwitch(bool pressureSwitch) {
    HALSIM_SetPCMPressureSwitch(m_index, pressureSwitch);
  }

  std::unique_ptr<CallbackStore> RegisterCompressorCurrentCallback(
      NotifyCallback callback, bool initialNotify) {
    auto store = std::make_unique<CallbackStore>(
        m_index, -1, callback, &HALSIM_CancelPCMCompressorCurrentCallback);
    store->SetUid(HALSIM_RegisterPCMCompressorCurrentCallback(
        m_index, &CallbackStoreThunk, store.get(), initialNotify));
    return store;
  }
  double GetCompressorCurrent() {
    return HALSIM_GetPCMCompressorCurrent(m_index);
  }
  void SetCompressorCurrent(double compressorCurrent) {
    HALSIM_SetPCMCompressorCurrent(m_index, compressorCurrent);
  }

  void ResetData() { HALSIM_ResetPCMData(m_index); }

 private:
  int m_index;
};
}  // namespace sim
}  // namespace frc
#endif  // __FRC_ROBORIO__

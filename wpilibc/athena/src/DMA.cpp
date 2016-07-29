/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2016. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "DMA.h"

#include "HAL/HAL.h"

DMA::DMA() {
  int32_t status = 0;
  HAL_InitializeDMA(&status);
  if (status != 0) {
    wpi_setErrorWithContext(status, HAL_GetErrorMessage(status));
  }
}

DMA::~DMA() { HAL_FreeDMA(); }

DMA* DMA::GetInstance() {
  static DMA instance;
  return &instance;
}

void DMA::SetPause(bool pause) {
  if (StatusIsFatal()) return;
  int32_t status = 0;
  HAL_SetDMAPause(pause, &status);
  wpi_setErrorWithContext(status, HAL_GetErrorMessage(status));
}

void DMA::SetRate(int32_t cycles) {
  if (StatusIsFatal()) return;
  int32_t status = 0;
  HAL_SetDMARate(cycles, &status);
  wpi_setErrorWithContext(status, HAL_GetErrorMessage(status));
}

void DMA::Add(Encoder* encoder) {
  if (StatusIsFatal()) return;
  int32_t status = 0;
  HAL_AddDMAEncoder(encoder->m_encoder, &status);
  wpi_setErrorWithContext(status, HAL_GetErrorMessage(status));
}

void DMA::Add(Counter* counter) {
  if (StatusIsFatal()) return;
  int32_t status = 0;
  HAL_AddDMACounter(counter->m_counter, &status);
  wpi_setErrorWithContext(status, HAL_GetErrorMessage(status));
}

void DMA::Add(DigitalSource* digitalSource) {
  if (StatusIsFatal()) return;
  int32_t status = 0;
  HAL_AddDMADigitalSource(digitalSource->GetPortHandleForRouting(), &status);
  wpi_setErrorWithContext(status, HAL_GetErrorMessage(status));
}

void DMA::Add(AnalogInput* analogInput) {
  if (StatusIsFatal()) return;
  int32_t status = 0;
  HAL_AddDMAAnalogInput(analogInput->m_port, &status);
  wpi_setErrorWithContext(status, HAL_GetErrorMessage(status));
}

void DMA::SetExternalTrigger(DigitalSource* input, bool rising, bool falling) {
  if (StatusIsFatal()) return;
  int32_t status = 0;
  HAL_SetDMAExternalTrigger(
      input->GetPortHandleForRouting(),
      (HAL_AnalogTriggerType)input->GetAnalogTriggerTypeForRouting(), rising,
      falling, &status);
  wpi_setErrorWithContext(status, HAL_GetErrorMessage(status));
}

void DMA::Start(int32_t queueDepth) {
  if (StatusIsFatal()) return;
  int32_t status = 0;
  HAL_StartDMA(queueDepth, &status);
  wpi_setErrorWithContext(status, HAL_GetErrorMessage(status));
}

DMA::ReadStatus DMA::Read(DMASample* sample, int32_t timeoutMs,
                          int32_t* remainingOut) {
  if (StatusIsFatal()) return STATUS_ERROR;
  int32_t status = 0;
  auto retVal = HAL_ReadDMA(sample->m_handle, timeoutMs, remainingOut, &status);
  wpi_setErrorWithContext(status, HAL_GetErrorMessage(status));
  // Sample needs DMA pointer to properly set status reports
  sample->m_dma = this;
  return (ReadStatus)retVal;
}

const char* DMA::NameOfReadStatus(ReadStatus s) {
  switch (s) {
    case STATUS_OK:
      return "OK";
    case STATUS_TIMEOUT:
      return "TIMEOUT";
    case STATUS_ERROR:
      return "ERROR";
    default:
      return "(bad ReadStatus code)";
  }
}

DMASample::DMASample() {
  int32_t status = 0;
  m_handle = HAL_MakeDMASample(&status);
  wpi_setGlobalErrorWithContext(status, HAL_GetErrorMessage(status));
}

DMASample::~DMASample() { HAL_FreeDMASample(m_handle); }

int32_t DMASample::GetTime() const {
  if (m_dma == nullptr) return 0;
  int32_t status = 0;
  int32_t retVal = HAL_GetDMASampleTime(m_handle, &status);
  wpi_setStaticErrorWithContext(m_dma, status, HAL_GetErrorMessage(status));
  return retVal;
}

double DMASample::GetTimestamp() const {
  if (m_dma == nullptr) return 0;
  int32_t status = 0;
  double retVal = HAL_GetDMASampleTimestamp(m_handle, &status);
  wpi_setStaticErrorWithContext(m_dma, status, HAL_GetErrorMessage(status));
  return retVal;
}

bool DMASample::Get(DigitalSource* input) const {
  if (m_dma == nullptr) return 0;
  int32_t status = 0;
  bool retVal = HAL_GetDMASampleDigitalSource(
      m_handle, input->GetPortHandleForRouting(), &status);
  wpi_setStaticErrorWithContext(m_dma, status, HAL_GetErrorMessage(status));
  return retVal;
}

int32_t DMASample::Get(Encoder* input) const {
  if (m_dma == nullptr) return 0;
  int32_t status = 0;
  int32_t retVal = HAL_GetDMASampleEncoder(m_handle, input->m_encoder, &status);
  wpi_setStaticErrorWithContext(m_dma, status, HAL_GetErrorMessage(status));
  return retVal;
}

int32_t DMASample::GetRaw(Encoder* input) const {
  if (m_dma == nullptr) return 0;
  int32_t status = 0;
  int32_t retVal =
      HAL_GetDMASampleEncoderRaw(m_handle, input->m_encoder, &status);
  wpi_setStaticErrorWithContext(m_dma, status, HAL_GetErrorMessage(status));
  return retVal;
}

int32_t DMASample::Get(Counter* input) const {
  if (m_dma == nullptr) return 0;
  int32_t status = 0;
  int32_t retVal = HAL_GetDMASampleCounter(m_handle, input->m_counter, &status);
  wpi_setStaticErrorWithContext(m_dma, status, HAL_GetErrorMessage(status));
  return retVal;
}

int32_t DMASample::GetValue(AnalogInput* input) const {
  if (m_dma == nullptr) return 0;
  int32_t status = 0;
  int32_t retVal =
      HAL_GetDMASampleAnalogInputRaw(m_handle, input->m_port, &status);
  wpi_setStaticErrorWithContext(m_dma, status, HAL_GetErrorMessage(status));
  return retVal;
}

double DMASample::GetVoltage(AnalogInput* input) const {
  if (m_dma == nullptr) return 0;
  int32_t status = 0;
  double_t retVal =
      HAL_GetDMASampleAnalogInputVoltage(m_handle, input->m_port, &status);
  wpi_setStaticErrorWithContext(m_dma, status, HAL_GetErrorMessage(status));
  return retVal;
}

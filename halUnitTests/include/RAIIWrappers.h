#pragma once

#include "HAL/HAL.h"

#include <iostream>

struct InterruptHandle { 
 public:

  InterruptHandle(HAL_Bool watcher, int32_t* status) {
    handle = HAL_InitializeInterrupts(watcher, status);
  }

  ~InterruptHandle() { 
    int32_t status = 0;
    HAL_CleanInterrupts(handle, &status);
  }

  operator HAL_InterruptHandle() const { return handle; }
 private:
  HAL_InterruptHandle handle = 0;
};

struct DIOHandle { 
 public:

  DIOHandle() {}

  DIOHandle(int32_t port, HAL_Bool input, int32_t* status) {
    handle = HAL_InitializeDIOPort(HAL_GetPort(port), input, status);
  }

  ~DIOHandle() { 
    HAL_FreeDIOPort(handle);
  }

  operator HAL_DigitalHandle() const { return handle; }
 private:
  HAL_DigitalHandle handle = 0;
};

struct PWMHandle { 
 public:

  PWMHandle() {}

  PWMHandle(int32_t port, int32_t* status) {
    handle = HAL_InitializePWMPort(HAL_GetPort(port), status);
  }

  ~PWMHandle() {
    int32_t status = 0;
    HAL_FreePWMPort(handle, &status);
  }

  operator HAL_DigitalHandle() const { return handle; }
 private:
  HAL_DigitalHandle handle = 0;
};

struct SolenoidHandle { 
 public:

  SolenoidHandle() {}

  SolenoidHandle(int32_t module, int32_t port, int32_t* status) {
    handle = HAL_InitializeSolenoidPort(HAL_GetPortWithModule(module, port), status);
  }

  ~SolenoidHandle() {
    HAL_FreeSolenoidPort(handle);
  }

  operator HAL_SolenoidHandle() const { return handle; }
 private:
  HAL_SolenoidHandle handle = 0;
};
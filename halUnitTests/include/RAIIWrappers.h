#pragma once

#include "HAL/HAL.h"

#include <iostream>

struct InterruptHandle { 
 public:

  InterruptHandle(HAL_Bool watcher, int32_t* status) {
    handle = HAL_InitializeInterrupts(watcher, status);
  }
  InterruptHandle(const InterruptHandle&) = delete;
  InterruptHandle operator=(const InterruptHandle&) =
      delete;

  InterruptHandle(InterruptHandle&&) = default;
  InterruptHandle& operator=(InterruptHandle&&) =
      default;

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
  DIOHandle(const DIOHandle&) = delete;
  DIOHandle operator=(const DIOHandle&) =
      delete;

  DIOHandle(DIOHandle&&) = default;
  DIOHandle& operator=(DIOHandle&&) =
      default;

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
  PWMHandle(const PWMHandle&) = delete;
  PWMHandle operator=(const PWMHandle&) =
      delete;

  PWMHandle(PWMHandle&&) = default;
  PWMHandle& operator=(PWMHandle&&) =
      default;

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
  SolenoidHandle(const SolenoidHandle&) = delete;
  SolenoidHandle operator=(const SolenoidHandle&) =
      delete;

  SolenoidHandle(SolenoidHandle&&) = default;
  SolenoidHandle& operator=(SolenoidHandle&&) =
      default;

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
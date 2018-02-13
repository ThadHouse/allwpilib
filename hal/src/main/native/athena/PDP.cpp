/*----------------------------------------------------------------------------*/
/* Copyright (c) 2016-2018 FIRST. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "HAL/PDP.h"

#include <memory>

#include "HAL/CANAPI.h"
#include "HAL/Errors.h"
#include "HAL/Ports.h"
#include "HAL/cpp/make_unique.h"
#include "HAL/handles/IndexedHandleResource.h"
#include "PortsInternal.h"

using namespace hal;

static constexpr HAL_CANManufacturer manufacturer =
    HAL_CANManufacturer::HAL_CAN_Man_kCTRE;

static constexpr HAL_CANDeviceType deviceType =
    HAL_CANDeviceType::HAL_CAN_Dev_kPowerDistribution;

static constexpr int32_t Status1 = 0x1400;
static constexpr int32_t Status2 = 0x1440;
static constexpr int32_t Status3 = 0x1480;
static constexpr int32_t StatusEnergy = 0x1740;

static constexpr int32_t Control1 = 0x1C00;

static constexpr int32_t TimeoutMs = 50;

/* encoder/decoders */
union PdpStatus1 {
  uint8_t data[8];
  struct {
    unsigned chan1_h8 : 8;
    unsigned chan2_h6 : 6;
    unsigned chan1_l2 : 2;
    unsigned chan3_h4 : 4;
    unsigned chan2_l4 : 4;
    unsigned chan4_h2 : 2;
    unsigned chan3_l6 : 6;
    unsigned chan4_l8 : 8;
    unsigned chan5_h8 : 8;
    unsigned chan6_h6 : 6;
    unsigned chan5_l2 : 2;
    unsigned reserved4 : 4;
    unsigned chan6_l4 : 4;
  };
};

union PdpStatus2 {
  uint8_t data[8];
  struct {
    unsigned chan7_h8 : 8;
    unsigned chan8_h6 : 6;
    unsigned chan7_l2 : 2;
    unsigned chan9_h4 : 4;
    unsigned chan8_l4 : 4;
    unsigned chan10_h2 : 2;
    unsigned chan9_l6 : 6;
    unsigned chan10_l8 : 8;
    unsigned chan11_h8 : 8;
    unsigned chan12_h6 : 6;
    unsigned chan11_l2 : 2;
    unsigned reserved4 : 4;
    unsigned chan12_l4 : 4;
  };
};

union PdpStatus3 {
  uint8_t data[8];
  struct {
    unsigned chan13_h8 : 8;
    unsigned chan14_h6 : 6;
    unsigned chan13_l2 : 2;
    unsigned chan15_h4 : 4;
    unsigned chan14_l4 : 4;
    unsigned chan16_h2 : 2;
    unsigned chan15_l6 : 6;
    unsigned chan16_l8 : 8;
    unsigned internalResBattery_mOhms : 8;
    unsigned busVoltage : 8;
    unsigned temp : 8;
  };
};

union PdpStatusEnergy {
  uint8_t data[8];
  struct {
    unsigned TmeasMs_likelywillbe20ms_ : 8;
    unsigned TotalCurrent_125mAperunit_h8 : 8;
    unsigned Power_125mWperunit_h4 : 4;
    unsigned TotalCurrent_125mAperunit_l4 : 4;
    unsigned Power_125mWperunit_m8 : 8;
    unsigned Energy_125mWPerUnitXTmeas_h4 : 4;
    unsigned Power_125mWperunit_l4 : 4;
    unsigned Energy_125mWPerUnitXTmeas_mh8 : 8;
    unsigned Energy_125mWPerUnitXTmeas_ml8 : 8;
    unsigned Energy_125mWPerUnitXTmeas_l8 : 8;
  };
};

static IndexedHandleResource<HAL_PDPHandle, HAL_CANHandle, kNumPDPModules,
                             HAL_HandleEnum::CAN>* pdpHandles;

namespace hal {
namespace init {
void InitializePDP() {
  static IndexedHandleResource<HAL_PDPHandle, HAL_CANHandle, kNumPDPModules,
                               HAL_HandleEnum::CAN>
      pH;
  pdpHandles = &pH;
}
}  // namespace init
}  // namespace hal

extern "C" {

HAL_PDPHandle HAL_InitializePDP(int32_t module, int32_t* status) {
  if (!HAL_CheckPDPModule(module)) {
    *status = PARAMETER_OUT_OF_RANGE;
    return HAL_kInvalidHandle;
  }

  auto handle = HAL_InitializeCAN(manufacturer, module, deviceType, status);

  if (*status != 0) {
    HAL_CleanCAN(handle);
    return HAL_kInvalidHandle;
  }

  return handle;
}

HAL_Bool HAL_CheckPDPModule(int32_t module) {
  return module < kNumPDPModules && module >= 0;
}

HAL_Bool HAL_CheckPDPChannel(int32_t channel) {
  return channel < kNumPDPChannels && channel >= 0;
}

double HAL_GetPDPTemperature(HAL_PDPHandle handle, int32_t* status) {
  PdpStatus3 pdpStatus;
  int32_t length = 0;
  uint64_t receivedTimestamp = 0;

  HAL_ReadCANPacketTimeout(handle, Status3, pdpStatus.data, &length,
                           &receivedTimestamp, TimeoutMs, status);

  return pdpStatus.temp * 1.03250836957542 - 67.8564500484966;
}

double HAL_GetPDPVoltage(HAL_PDPHandle handle, int32_t* status) {
  PdpStatus3 pdpStatus;
  int32_t length = 0;
  uint64_t receivedTimestamp = 0;

  HAL_ReadCANPacketTimeout(handle, Status3, pdpStatus.data, &length,
                           &receivedTimestamp, TimeoutMs, status);

  return pdpStatus.busVoltage * 0.05 + 4.0;
}

double HAL_GetPDPChannelCurrent(HAL_PDPHandle handle, int32_t channel,
                                int32_t* status) {
  if (!checkPDPInit(module, status)) return 0;

  double current;

  *status = pdp[module]->GetChannelCurrent(channel, current);

  return current;
}

double HAL_GetPDPTotalCurrent(HAL_PDPHandle handle, int32_t* status) {
  if (!checkPDPInit(module, status)) return 0;

  double current;

  *status = pdp[module]->GetTotalCurrent(current);

  return current;
}

double HAL_GetPDPTotalPower(HAL_PDPHandle handle, int32_t* status) {
  if (!checkPDPInit(module, status)) return 0;

  double power;

  *status = pdp[module]->GetTotalPower(power);

  return power;
}

double HAL_GetPDPTotalEnergy(HAL_PDPHandle handle, int32_t* status) {
  if (!checkPDPInit(module, status)) return 0;

  double energy;

  *status = pdp[module]->GetTotalEnergy(energy);

  return energy;
}

void HAL_ResetPDPTotalEnergy(HAL_PDPHandle handle, int32_t* status) {
  if (!checkPDPInit(module, status)) return;

  *status = pdp[module]->ResetEnergy();
}

void HAL_ClearPDPStickyFaults(HAL_PDPHandle handle, int32_t* status) {
  if (!checkPDPInit(module, status)) return;

  *status = pdp[module]->ClearStickyFaults();
}

}  // extern "C"

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

static constexpr int32_t Status1 = 0x50;
static constexpr int32_t Status2 = 0x51;
static constexpr int32_t Status3 = 0x52;
static constexpr int32_t StatusEnergy = 0x5D;

static constexpr int32_t Control1 = 0x70;

static constexpr int32_t TimeoutMs = 50;
static constexpr int32_t StatusPeriodMs = 25;

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

namespace hal {
namespace init {
void InitializePDP() {}
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

void HAL_CleanPDP(HAL_PDPHandle handle) { HAL_CleanCAN(handle); }

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

  HAL_ReadCANPeriodicPacket(handle, Status3, pdpStatus.data, &length,
                            &receivedTimestamp, TimeoutMs, StatusPeriodMs,
                            status);

  return pdpStatus.temp * 1.03250836957542 - 67.8564500484966;
}

double HAL_GetPDPVoltage(HAL_PDPHandle handle, int32_t* status) {
  PdpStatus3 pdpStatus;
  int32_t length = 0;
  uint64_t receivedTimestamp = 0;

  HAL_ReadCANPeriodicPacket(handle, Status3, pdpStatus.data, &length,
                            &receivedTimestamp, TimeoutMs, StatusPeriodMs,
                            status);

  return pdpStatus.busVoltage * 0.05 + 4.0;
}

double HAL_GetPDPChannelCurrent(HAL_PDPHandle handle, int32_t channel,
                                int32_t* status) {
  if (!HAL_CheckPDPChannel(channel)) {
    *status = PARAMETER_OUT_OF_RANGE;
    return 0;
  }

  int32_t length = 0;
  uint64_t receivedTimestamp = 0;

  double raw = 0;

  if (channel <= 5) {
    PdpStatus1 pdpStatus;
    HAL_ReadCANPeriodicPacket(handle, Status1, pdpStatus.data, &length,
                              &receivedTimestamp, TimeoutMs, StatusPeriodMs,
                              status);
    switch (channel) {
      case 0:
        raw = (static_cast<uint32_t>(pdpStatus.chan1_h8) << 2) |
              pdpStatus.chan1_l2;
        break;
      case 1:
        raw = (static_cast<uint32_t>(pdpStatus.chan2_h6) << 4) |
              pdpStatus.chan2_l4;
        break;
      case 2:
        raw = (static_cast<uint32_t>(pdpStatus.chan3_h4) << 6) |
              pdpStatus.chan3_l6;
        break;
      case 3:
        raw = (static_cast<uint32_t>(pdpStatus.chan4_h2) << 8) |
              pdpStatus.chan4_l8;
        break;
      case 4:
        raw = (static_cast<uint32_t>(pdpStatus.chan5_h8) << 2) |
              pdpStatus.chan5_l2;
        break;
      case 5:
        raw = (static_cast<uint32_t>(pdpStatus.chan6_h6) << 4) |
              pdpStatus.chan6_l4;
        break;
    }
  } else if (channel <= 11) {
    PdpStatus2 pdpStatus;
    HAL_ReadCANPeriodicPacket(handle, Status2, pdpStatus.data, &length,
                              &receivedTimestamp, TimeoutMs, StatusPeriodMs,
                              status);
    switch (channel) {
      case 6:
        raw = (static_cast<uint32_t>(pdpStatus.chan7_h8) << 2) |
              pdpStatus.chan7_l2;
        break;
      case 7:
        raw = (static_cast<uint32_t>(pdpStatus.chan8_h6) << 4) |
              pdpStatus.chan8_l4;
        break;
      case 8:
        raw = (static_cast<uint32_t>(pdpStatus.chan9_h4) << 6) |
              pdpStatus.chan9_l6;
        break;
      case 9:
        raw = (static_cast<uint32_t>(pdpStatus.chan10_h2) << 8) |
              pdpStatus.chan10_l8;
        break;
      case 10:
        raw = (static_cast<uint32_t>(pdpStatus.chan11_h8) << 2) |
              pdpStatus.chan11_l2;
        break;
      case 11:
        raw = (static_cast<uint32_t>(pdpStatus.chan12_h6) << 4) |
              pdpStatus.chan12_l4;
        break;
    }
  } else {
    PdpStatus3 pdpStatus;
    HAL_ReadCANPeriodicPacket(handle, Status3, pdpStatus.data, &length,
                              &receivedTimestamp, TimeoutMs, StatusPeriodMs,
                              status);
    switch (channel) {
      case 12:
        raw = (static_cast<uint32_t>(pdpStatus.chan13_h8) << 2) |
              pdpStatus.chan13_l2;
        break;
      case 13:
        raw = (static_cast<uint32_t>(pdpStatus.chan14_h6) << 4) |
              pdpStatus.chan14_l4;
        break;
      case 14:
        raw = (static_cast<uint32_t>(pdpStatus.chan15_h4) << 6) |
              pdpStatus.chan15_l6;
        break;
      case 15:
        raw = (static_cast<uint32_t>(pdpStatus.chan16_h2) << 8) |
              pdpStatus.chan16_l8;
        break;
    }
  }

  return raw * 0.125;
}

double HAL_GetPDPTotalCurrent(HAL_PDPHandle handle, int32_t* status) {
  PdpStatusEnergy pdpStatus;
  int32_t length = 0;
  uint64_t receivedTimestamp = 0;

  HAL_ReadCANPeriodicPacket(handle, StatusEnergy, pdpStatus.data, &length,
                            &receivedTimestamp, TimeoutMs, StatusPeriodMs,
                            status);

  uint32_t raw;
  raw = pdpStatus.TotalCurrent_125mAperunit_h8;
  raw <<= 4;
  raw |= pdpStatus.TotalCurrent_125mAperunit_l4;
  return 0.125 * raw;
}

double HAL_GetPDPTotalPower(HAL_PDPHandle handle, int32_t* status) {
  PdpStatusEnergy pdpStatus;
  int32_t length = 0;
  uint64_t receivedTimestamp = 0;

  HAL_ReadCANPeriodicPacket(handle, StatusEnergy, pdpStatus.data, &length,
                            &receivedTimestamp, TimeoutMs, StatusPeriodMs,
                            status);

  uint32_t raw;
  raw = pdpStatus.Power_125mWperunit_h4;
  raw <<= 8;
  raw |= pdpStatus.Power_125mWperunit_m8;
  raw <<= 4;
  raw |= pdpStatus.Power_125mWperunit_l4;
  return 0.125 * raw;
}

double HAL_GetPDPTotalEnergy(HAL_PDPHandle handle, int32_t* status) {
  PdpStatusEnergy pdpStatus;
  int32_t length = 0;
  uint64_t receivedTimestamp = 0;

  HAL_ReadCANPeriodicPacket(handle, StatusEnergy, pdpStatus.data, &length,
                            &receivedTimestamp, TimeoutMs, StatusPeriodMs,
                            status);

  uint32_t raw;
  raw = pdpStatus.Energy_125mWPerUnitXTmeas_h4;
  raw <<= 8;
  raw |= pdpStatus.Energy_125mWPerUnitXTmeas_mh8;
  raw <<= 8;
  raw |= pdpStatus.Energy_125mWPerUnitXTmeas_ml8;
  raw <<= 8;
  raw |= pdpStatus.Energy_125mWPerUnitXTmeas_l8;

  double energyJoules = 0.125 * raw; /* mW integrated every TmeasMs */
  energyJoules *= 0.001;             /* convert from mW to W */
  energyJoules *=
      pdpStatus.TmeasMs_likelywillbe20ms_; /* multiplied by TmeasMs = joules */
  return 0.125 * raw;
}

void HAL_ResetPDPTotalEnergy(HAL_PDPHandle handle, int32_t* status) {
  uint8_t pdpControl[] = {0x40}; /* only bit set is ResetEnergy */
  HAL_WriteCANPacket(handle, pdpControl, 1, Control1, status);
}

void HAL_ClearPDPStickyFaults(HAL_PDPHandle handle, int32_t* status) {
  uint8_t pdpControl[] = {0x80}; /* only bit set is ClearStickyFaults */
  HAL_WriteCANPacket(handle, pdpControl, 1, Control1, status);
}

}  // extern "C"

/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2016. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#pragma once

#include "stdint.h"

#include "AnalogInput.h"
#include "Counter.h"
#include "DigitalSource.h"
#include "Encoder.h"

class DMA;

class DMASample {
 public:
  DMASample();
  ~DMASample();

  // Returns the FPGA timestamp of the sample in seconds.
  double GetTimestamp() const;
  // Returns the FPGA timestamp of the sample in microseconds.
  int32_t GetTime() const;

  // All Get methods either return the requested value, or set the Error.

  // Returns the value of the digital input in the sample.
  bool Get(DigitalSource* input) const;
  // Returns the raw value of the encoder in the sample.
  int32_t GetRaw(Encoder* input) const;
  // Returns the {1, 2, or 4} X scaled value of the encoder in the sample.
  int32_t Get(Encoder* input) const;
  int32_t Get(Counter* counter) const;
  // Returns the raw 12-bit value from the ADC.
  int32_t GetValue(AnalogInput* input) const;
  // Returns the scaled value of an analog input.
  double GetVoltage(AnalogInput* input) const;

 private:
  friend DMA;

  DMA* m_dma;
  HAL_DMASampleHandle m_handle = HAL_kInvalidHandle;
};

// TODO(austin): ErrorBase...
class DMA : public ErrorBase {
 public:
  virtual ~DMA();

  static DMA* GetInstance();

  // Sets whether or not DMA is paused.
  // If not specified, the default is false.
  void SetPause(bool pause);

  // Sets the number of triggers that need to occur before a sample is saved.
  // If not specified, the default is 1.
  void SetRate(int32_t cycles);

  // Adds the input signal to the state to snapshot on the trigger event.
  // It is safe to add the same input multiple times, but there is currently
  // no way to remove one once it has been added.
  // Call Add() and SetExternalTrigger() before Start().
  void Add(Encoder* encoder);
  void Add(Counter* counter);
  void Add(DigitalSource* input);
  void Add(AnalogInput* input);

  // Configures DMA to trigger on an external trigger.  There can only be 4
  // external triggers.
  // Call Add() and SetExternalTrigger() before Start().
  void SetExternalTrigger(DigitalSource* input, bool rising, bool falling);

  // Starts reading samples into the buffer.  Clears all previous samples before
  // starting.
  // Call Start() before Read().
  void Start(int32_t queue_depth);

  enum ReadStatus {
    STATUS_OK = 0,
    STATUS_TIMEOUT = 1,
    STATUS_ERROR = 2,
  };

  // Reads a sample from the DMA buffer, waiting up to timeout_ms for it.
  // Returns a status code indicating whether the read worked, timed out, or
  // failed.
  // Returns in *remaining_out the number of DMA samples still queued after this
  // Read().
  // Call Add() and SetExternalTrigger() then Start() before Read().
  // The sample is only usable while this DMA object is left started.
  ReadStatus Read(DMASample* sample, int32_t timeout_ms,
                  int32_t* remaining_out);

  // Translates a ReadStatus code to a string name.
  static const char* NameOfReadStatus(ReadStatus s);

 private:
  DMA();
  friend DMASample;
};

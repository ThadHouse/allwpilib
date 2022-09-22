// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

#pragma once

#include "wpi/SmallVector.h"
#include "wpi/Synchronization.h"
#include "wpi/mutex.h"

namespace wpi {
struct EventVector {
  wpi::mutex mutex;
  wpi::SmallVector<WPI_EventHandle, 4> events;

  void Add(WPI_EventHandle handle) {
    std::scoped_lock lock{mutex};
    events.emplace_back(handle);
  }
  void Remove(WPI_EventHandle handle) {
    std::scoped_lock lock{mutex};
    auto it = std::find_if(
        events.begin(), events.end(),
        [=](const WPI_EventHandle fromHandle) { return fromHandle == handle; });
    if (it != events.end()) {
      events.erase(it);
    }
  }
  void Wakeup() {
    std::scoped_lock lock{mutex};
    for (auto&& handle : events) {
      wpi::SetEvent(handle);
    }
  }
};
}  // namespace wpi
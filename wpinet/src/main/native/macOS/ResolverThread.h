// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

#if defined(__APPLE__)

#pragma once

#include <netinet/in.h>
#include <poll.h>

#include <atomic>
#include <memory>
#include <thread>
#include <utility>
#include <vector>

#include <wpi/Synchronization.h>
#include <wpi/mutex.h>

#include "dns_sd.h"

namespace wpi {
class ResolverThread {
 private:
  struct private_init {};

 public:
  explicit ResolverThread(const private_init&);
  ~ResolverThread() noexcept;

  void AddServiceRef(std::function<void(DNSServiceRef)> onAdd);

  void RemoveServiceRef(std::function<void()> onRemove);

  static std::shared_ptr<ResolverThread> Get();

 private:
  void ThreadMain();

  bool AddQueueRef();
  void RemoveQueueRef();

  wpi::mutex serviceRefMutex;
  std::thread thread;
  int queue{-1};
  int numReferences{0};
  DNSServiceRef globalRef{nullptr};
  static constexpr uintptr_t shutdownHandle = 0x100000042;
  static constexpr uintptr_t addHandle = 0x100000043;
  static constexpr uintptr_t removeHandle = 0x100000044;
};
}  // namespace wpi

#endif  // defined(__APPLE__)

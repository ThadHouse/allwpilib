// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

#if defined(__APPLE__)

#include "ResolverThread.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include <wpi/mutex.h>

#include "sys/event.h"

using namespace wpi;

ResolverThread::ResolverThread(const private_init&) {}

ResolverThread::~ResolverThread() noexcept {
  // The thread at this point should be cleaned up, as there are no more refs.
  // If there are refs, we can't do anything.
}

bool ResolverThread::AddQueueRef(DNSServiceRef serviceRef, dnssd_sock_t sock) {
  std::scoped_lock lock{serviceRefMutex};
  numReferences++;
  bool needsThreadCreated = numReferences == 1;
  if (numReferences == 1) {
    // Create queue
    queue = kqueue();
    if (queue < 0) {
      return false;
    }
  }

  // We add the event here so we never have to deal with the scenario of having an empty kqueue.
  struct kevent sockEvent = {};
  EV_SET(&sockEvent, sock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, serviceRef);
  int result = kevent(queue, &sockEvent, 1, nullptr, 0, nullptr);
  if (result < 0) {
    close(queue);
    queue = -1;
    return false;
  }

  if (needsThreadCreated) {
    thread = std::thread([this] { ThreadMain(); });
  }

  return true;
}

void ResolverThread::RemoveQueueRef() {
  std::scoped_lock lock{serviceRefMutex};
  numReferences--;
  if (numReferences == 0) {
    // Cleanup
    // Fire into the queue
    struct kevent stopEvent = {};
    EV_SET(&stopEvent, shutdownHandle, EVFILT_USER, EV_ADD | EV_ONESHOT,
           NOTE_TRIGGER, 0, nullptr);
    // If we get an error here, we don't have anything we can do.
    // We will either deadlock or create a massive use after free.
    (void)kevent(queue, &stopEvent, 1, nullptr, 0, nullptr);

    if (thread.joinable()) {
      thread.join();
    }
  }
}

std::optional<dnssd_sock_t> ResolverThread::AddServiceRef(
    DNSServiceRef serviceRef) {
  dnssd_sock_t sock = DNSServiceRefSockFD(serviceRef);
  if (sock == -1) {
    return {};
  }

  if (!AddQueueRef(serviceRef, sock)) {
    return {};
  }

  return sock;
}

using ShutdownDataStore = std::pair<wpi::Event&, DNSServiceRef>;

void ResolverThread::RemoveServiceRef(dnssd_sock_t serviceShutdownHandle,
                                      DNSServiceRef serviceRef) {
  wpi::Event shutdownEvent{true, false};
  ShutdownDataStore dataStore{shutdownEvent, serviceRef};

  // Fire into the queue
  struct kevent stopEvent = {};
  EV_SET(&stopEvent, serviceShutdownHandle, EVFILT_USER, EV_ADD | EV_ONESHOT,
         NOTE_TRIGGER, 0, &dataStore);
  // If we get an error here, we don't have anything we can do.
  // We will either deadlock or create a massive use after free.
  // We're choosing the deadlock.
  (void)kevent(queue, &stopEvent, 1, nullptr, 0, nullptr);

  wpi::WaitForObject(shutdownEvent.GetHandle());

  RemoveQueueRef();
}

void ResolverThread::ThreadMain() {
  constexpr int numEventsToProcess = 16;
  struct kevent events[numEventsToProcess];
  while (true) {
    int numEvents = kevent(queue, NULL, 0, events, numEventsToProcess, nullptr);

    if (numEvents < 0) {
      // Error in the loop, exit.
      // TODO figure out if this breaks the world.
      break;
    }

    for (int i = 0; i < numEvents; i++) {
      auto& event = events[i];
      if (event.filter == EVFILT_USER) {
        // User event.
        if (event.ident == shutdownHandle) {
          // This thread is being shut down. We're done here.
          return;
        }

        ShutdownDataStore& dataStore =
            *reinterpret_cast<ShutdownDataStore*>(event.udata);
        dnssd_sock_t sock = event.ident;

        // Delete the event from the queue
        struct kevent deleteEvent = {};
        EV_SET(&deleteEvent, sock, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
        (void)kevent(queue, &deleteEvent, 1, nullptr, 0, nullptr);

        // Deallocate the service ref
        DNSServiceRefDeallocate(dataStore.second);

        // If we happen to have this service ref in any other events, remove it.
        for (int j = 0; j < numEvents; j++) {
          if (event.filter == EVFILT_READ &&
              event.ident == static_cast<uintptr_t>(sock)) {
            event.filter = 0;
          }
        }

        dataStore.first.Set();

      } else if (event.filter == EVFILT_READ) {
        // We're getting a read event. Just process it.
        DNSServiceProcessResult(reinterpret_cast<DNSServiceRef>(event.udata));
      }
    }
  }

  // std::vector<pollfd> readSockets;
  // std::vector<DNSServiceRef> serviceRefs;

  // while (running) {
  //   readSockets.clear();
  //   serviceRefs.clear();

  //   for (auto&& i : this->serviceRefs) {
  //     readSockets.emplace_back(pollfd{i.second, POLLIN, 0});
  //     serviceRefs.emplace_back(i.first);
  //   }

  //   int res = poll(readSockets.begin().base(), readSockets.size(), 100);

  //   if (res > 0) {
  //     for (size_t i = 0; i < readSockets.size(); i++) {
  //       if (readSockets[i].revents == POLLIN) {
  //         DNSServiceProcessResult(serviceRefs[i]);
  //       }
  //     }
  //   } else if (res == 0) {
  //     if (!running) {
  //       CleanupRefs();
  //       break;
  //     }
  //   }

  //   if (CleanupRefs()) {
  //     break;
  //   }
  // }
}

static wpi::mutex ThreadLoopLock;
static std::weak_ptr<ResolverThread> ThreadLoop;

std::shared_ptr<ResolverThread> ResolverThread::Get() {
  std::scoped_lock lock{ThreadLoopLock};
  auto locked = ThreadLoop.lock();
  if (!locked) {
    locked = std::make_unique<ResolverThread>(private_init{});
    ThreadLoop = locked;
  }
  return locked;
}

#endif  // defined(__APPLE__)

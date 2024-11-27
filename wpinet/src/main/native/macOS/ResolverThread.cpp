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

bool ResolverThread::AddQueueRef() {
  std::scoped_lock lock{serviceRefMutex};
  numReferences++;
  if (numReferences == 1) {
    // Create queue
    queue = kqueue();
    if (queue < 0) {
      numReferences--;
      return false;
    }

    globalRef = nullptr;
    DNSServiceErrorType status = DNSServiceCreateConnection(&globalRef);
    if (status != kDNSServiceErr_NoError) {
      globalRef = nullptr;
      close(queue);
      queue = -1;
      numReferences--;
      return false;
    }

    dnssd_sock_t sock = DNSServiceRefSockFD(globalRef);
    if (sock == -1) {
      DNSServiceRefDeallocate(globalRef);
      globalRef = nullptr;
      close(queue);
      queue = -1;
      numReferences--;
      return false;
    }

    struct kevent sockEvent = {};
    EV_SET(&sockEvent, sock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);
    int result = kevent(queue, &sockEvent, 1, nullptr, 0, nullptr);
    if (result < 0) {
      DNSServiceRefDeallocate(globalRef);
      globalRef = nullptr;
      close(queue);
      queue = -1;
      numReferences--;
      return false;
    }

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

    DNSServiceRefDeallocate(globalRef);
    globalRef = nullptr;
  }
}

using AddRefDataStore =
    std::pair<wpi::Event&, std::function<void(DNSServiceRef)>&>;

void ResolverThread::AddServiceRef(std::function<void(DNSServiceRef)> onAdd) {
  if (!AddQueueRef()) {
    return;
  }

  wpi::Event addEvent{true, false};
  AddRefDataStore dataStore{addEvent, onAdd};

  // Fire into the queue
  struct kevent addKEvent = {};
  EV_SET(&addKEvent, addHandle, EVFILT_USER, EV_ADD | EV_ONESHOT, NOTE_TRIGGER,
         0, &dataStore);
  // If we get an error here, we don't have anything we can do.
  // We will either deadlock or create a massive use after free.
  // We're choosing the deadlock.
  (void)kevent(queue, &addKEvent, 1, nullptr, 0, nullptr);

  wpi::WaitForObject(addEvent.GetHandle());
}

using ShutdownDataStore = std::pair<wpi::Event&, std::function<void()>&>;

void ResolverThread::RemoveServiceRef(std::function<void()> onRemove) {
  wpi::Event shutdownEvent{true, false};
  ShutdownDataStore dataStore{shutdownEvent, onRemove};

  // Fire into the queue
  struct kevent stopEvent = {};
  EV_SET(&stopEvent, removeHandle, EVFILT_USER, EV_ADD | EV_ONESHOT,
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
        } else if (event.ident == addHandle) {
          AddRefDataStore& dataStore =
              *reinterpret_cast<AddRefDataStore*>(event.udata);
          // Call up to delete any references
          dataStore.second(globalRef);
          dataStore.first.Set();
        } else if (event.ident == removeHandle) {
          ShutdownDataStore& dataStore =
              *reinterpret_cast<ShutdownDataStore*>(event.udata);
          // Call up to delete any references
          dataStore.second();
          dataStore.first.Set();
        }
      } else if (event.filter == EVFILT_READ) {
        // We're getting a read event. Just process it.
        DNSServiceProcessResult(globalRef);
      }
    }
  }
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

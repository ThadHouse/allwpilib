## 2027 Branch Bug Audit — Tracking Issue

Comprehensive code review of the 2027 branch to identify bugs that could cause crashes, data corruption, or incorrect behavior. Findings are organized by severity and component.

---

## 🔴 Critical Bugs

### 1. [wpiutil] Semaphore uses wrong backing vector (`eventIds` instead of `semaphoreIds`)
**Files:** `wpiutil/src/main/native/cpp/Synchronization.cpp` lines 183, 207

`DestroySemaphore()` erases from `manager.eventIds` instead of `manager.semaphoreIds`, so semaphore IDs are never freed (resource leak). `ReleaseSemaphore()` reads the max count from `manager.eventIds[index]` instead of `manager.semaphoreIds[index]`, reading uninitialized/wrong values and causing incorrect release logic.

```cpp
// Line 183 in DestroySemaphore — should be semaphoreIds
manager.eventIds.erase(handle & 0xffffff);

// Line 207 in ReleaseSemaphore — should be semaphoreIds
int maxCount = manager.eventIds[index];
```

**Impact:** Semaphore operations produce incorrect results; semaphore IDs leak; could cause silent data corruption in any code using WPI semaphores.

---

### 2. [commandsv2] Iterator invalidation in `CommandScheduler::Cancel()`
**File:** `commandsv2/src/main/native/cpp/frc2/command/CommandScheduler.cpp` lines 318–322

Erasing from a map during a range-based for loop invalidates the iterator, causing undefined behavior.

```cpp
for (auto&& requirement : m_impl->requirements) {
  if (requirement.second == command) {
    m_impl->requirements.erase(requirement.first); // UB: invalidates iterator
  }
}
```

**Impact:** Crash or memory corruption when canceling a command that requires multiple subsystems.

---

### 3. [wpilibc] Use-after-free in `DriverStation::Instance` destructor vs `RefreshData`
**File:** `wpilibc/src/main/native/cpp/driverstation/DriverStation.cpp` lines 221–224, 800–802

The destructor does `delete dataLogSender.load()` without synchronization against `RefreshData()` which loads and uses the same pointer concurrently. Between the `load()` in RefreshData and the `Send()` call, the destructor can delete the object.

```cpp
// Destructor (line 222-223)
if (dataLogSender) { delete dataLogSender.load(); }

// RefreshData (line 800-801) — can race with destructor
if (auto sender = inst.dataLogSender.load()) { sender->Send(...); }
```

**Impact:** Use-after-free crash during robot shutdown while driver station data is still being refreshed.

---

### 4. [wpilibc] Double-free / use-after-free in Notifier move operations
**File:** `wpilibc/src/main/native/cpp/system/Notifier.cpp` lines 84–97

Move constructor and move assignment use non-atomic load/store sequences on `m_notifier` without synchronization. The destructor uses `m_notifier.exchange(0)` atomically, but the move operations can race with it.

**Impact:** Double-free of HAL notifier handle if move and destruction race.

---

### 5. [ntcore] Use-after-destroy: `mpack_reader_remaining()` called after `mpack_reader_destroy()`
**File:** `ntcore/src/main/native/cpp/net/WireDecoder.cpp` lines 587, 596

```cpp
auto err = mpack_reader_destroy(&reader);  // Line 587
// ...
*in = wpi::util::take_back(*in, mpack_reader_remaining(&reader, nullptr));  // Line 596
```

Currently works because `init_data` sets no teardown and fields aren't zeroed, but violates the mpack API contract and will break if mpack internals change.

---

### 6. [ntcore] Missing mpack cleanup on error path in `WireDecodeBinary()`
**File:** `ntcore/src/main/native/cpp/net/WireDecoder.cpp` lines 582–584

The `default` case returns `false` without calling `mpack_done_array()` or `mpack_reader_destroy()`.

```cpp
default:
  *error = fmt::format("unrecognized type {}", type);
  return false;  // No cleanup of mpack_reader
```

---

## 🟠 High Severity Bugs

### 7. [wpiutil] Null pointer dereference in `WPI_WaitForObjectTimeout` / `WPI_WaitForObjectsTimeout`
**File:** `wpiutil/src/main/native/cpp/Synchronization.cpp` lines 446, 464

Both C API functions unconditionally dereference the `timed_out` pointer without null-checking, despite the header documenting it as "if non-null".

```cpp
*timed_out = timedOutBool ? 1 : 0;  // Crash if timed_out is NULL
```

---

### 8. [wpilibj] Unsynchronized `HashMap` in `SendableChooser.addOption()`
**File:** `wpilibj/src/main/java/org/wpilib/smartdashboard/SendableChooser.java` lines 74–75

`addOption()` writes to `m_map` without holding `m_mutex`, while `getSelected()` reads under lock. Concurrent modification of a `HashMap` is undefined behavior in Java (can cause infinite loops or `ConcurrentModificationException`).

```java
public void addOption(String name, V object) {
  m_map.put(name, object);  // No synchronization!
}
```

---

### 9. [wpilibc] Race condition in Watchdog move assignment
**File:** `wpilibc/src/main/native/cpp/system/Watchdog.cpp` lines 146–162

`m_impl = rhs.m_impl` is assigned before the lock is acquired. If two threads move Watchdog objects simultaneously, the shared watchdog list can be corrupted.

---

### 10. [wpilibc] Race condition in `EventLoop.m_running`
**File:** `wpilibc/src/main/native/cpp/event/EventLoop.cpp` lines 26, 34, 41

`m_running` is read in `Bind()` and `Clear()` without synchronization, while `Poll()` modifies it from another thread.

---

### 11. [cscore] Use-after-free in `HttpCameraImpl::DeviceConnect()`
**File:** `cscore/src/main/native/cpp/HttpCameraImpl.cpp` lines 180–236

A raw pointer `conn` is obtained from `unique_ptr` under lock, then the lock is released and `conn` is used. The monitor thread can set `m_streamConn = nullptr` concurrently, freeing the object.

```cpp
wpi::net::HttpConnection* conn = connPtr.get();
{ std::scoped_lock lock(m_mutex); m_streamConn = std::move(connPtr); }
// Lock released — conn can be freed by monitor thread
conn->Handshake(req, &warn);  // Potential use-after-free
```

---

### 12. [ntcore] Potential callback invalidation in `ListenerStorage` thread loop
**File:** `ntcore/src/main/native/cpp/ListenerStorage.cpp` lines 28–37

The callback is copied from the map and called outside the lock, but the `m_callbacks` map could be modified by another thread between finding the entry and copying the callback value.

---

### 13. [commandsv2] Potential null dereference in `DeferredCommand` (C++ and Java)
**Files:**
- `commandsv2/src/main/native/cpp/frc2/command/DeferredCommand.cpp` lines 22–33
- `commandsv2/src/main/java/org/wpilib/command2/DeferredCommand.java` lines 49–54

If the supplier returns null/empty, `m_command` methods are called unconditionally, causing null pointer dereference / NPE.

---

## 🟡 Medium Severity Bugs

### 14. [wpilibc/wpilibj] Data races on `m_reported` / `gReported` flags
**Files:**
- `wpilibc/src/main/native/cpp/drive/DifferentialDrive.cpp` lines 43–46, 65–68, 87–90
- `wpilibc/src/main/native/cpp/smartdashboard/SmartDashboard.cpp` lines 51, 78–91
- `wpilibj/src/main/java/org/wpilib/smartdashboard/SmartDashboard.java` lines 119–125
- `wpilibj/src/main/java/org/wpilib/drive/DifferentialDrive.java` lines 160–163
- `wpilibj/src/main/java/org/wpilib/drive/MecanumDrive.java` lines 197–199

Multiple `static bool reported` / instance `m_reported` flags are read and written without synchronization. Technically undefined behavior in C++ and a data race in Java, though impact is minor (extra `HAL_ReportUsage` calls).

---

### 15. [wpinet] Non-thread-safe static RNG in WebSocket code
**Files:**
- `wpinet/src/main/native/cpp/WebSocketSerializer.cpp` lines 64–65
- `wpinet/src/main/native/cpp/WebSocket.cpp` lines 118–119

`static std::default_random_engine gen` is accessed from multiple threads without synchronization. Can corrupt internal RNG state.

---

### 16. [wpinet] Potential integer underflow in WebSocket frame size calculation
**File:** `wpinet/src/main/native/cpp/WebSocket.cpp` line 545

```cpp
need = m_frameStart + m_frameSize - m_payload.size();  // unsigned arithmetic
```

If `m_payload.size()` exceeds `m_frameStart + m_frameSize`, unsigned wraparound produces a very large `need` value.

---

### 17. [wpimath] Division by zero in `TrajectoryParameterizer` time parameterization
**File:** `wpimath/src/main/native/cpp/trajectory/TrajectoryParameterizer.cpp` lines 185, 196

The time parameterization loop doesn't guard against `ds == 0` (zero-distance segments), unlike the forward/backward passes.

```cpp
accel = (state.maxVelocity * state.maxVelocity - v * v) / (ds * 2);  // div by zero if ds==0
```

**Impact:** NaN/infinity values in trajectory if consecutive waypoints are at the same position.

---

### 18. [wpilibc] Resource leak — `RobotBase` connection listener never removed
**File:** `wpilibc/src/main/native/cppcs/RobotBase.cpp` lines 216–222

The NT connection listener created in the constructor is never removed in the destructor (`virtual ~RobotBase() = default`).

---

### 19. [wpilibj] Lock management issues in `Watchdog.schedulerFunc()`
**File:** `wpilibj/src/main/java/org/wpilib/system/Watchdog.java` lines 276–278

The mutex is explicitly unlocked before the callback and re-locked after. If the callback throws, the re-lock never happens, leaving the mutex in an inconsistent state for the `finally` block.

```java
m_queueMutex.unlock();
watchdog.m_callback.run();  // If this throws...
m_queueMutex.lock();        // ...this never executes
```

---

## Summary

| Severity | Count |
|----------|-------|
| 🔴 Critical | 6 |
| 🟠 High | 7 |
| 🟡 Medium | 6 |
| **Total** | **19** |

The most impactful issues to fix first:
1. **Semaphore `eventIds`/`semaphoreIds` mix-up** (#1) — affects all semaphore operations
2. **Iterator invalidation in `CommandScheduler::Cancel()`** (#2) — can crash during command cancellation
3. **`SendableChooser` unsynchronized map** (#8) — can crash or hang in common robot code patterns
4. **Various concurrency bugs** in DriverStation, Notifier, HttpCamera — can cause use-after-free

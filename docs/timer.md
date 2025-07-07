# Interrupt Timer

The `InterruptTimer` is a dedicated thread that drives the main timing loop of the application.  
Every *N* milliseconds it enqueues two events into the global `events::queue`:

* `TimeElapsedEvent` – Informs interested subsystems (e.g. physics, animations, logic) how much real-time has passed since the previous tick.
* `DrawEvent` – Triggers a new render pass in the graphics subsystem.

By default the timer starts at 120 FPS (≈8.33 ms per tick), but you can now **change the frame-rate at run-time** without restarting the program.

---
## Changing the frame-rate at run-time
Send a `SetFrameRateEvent` with the desired FPS:

```cpp
#include "events/events.hpp"
...
// Increase to 240 FPS
events::queue.enqueue(std::make_shared<SetFrameRateEvent>(240));
```

Internally the `InterruptTimer` listens for this event and calls `setFrameRate(int fps)`, recalculating its sleep interval on-the-fly.  If you pass a value ≤ 0 the request will be ignored.

---
## Stopping the timer
The existing `StopEvent` is still supported.  Emitting it will gracefully stop the timer thread as before:

```cpp
events::queue.enqueue(std::make_shared<StopEvent>());
```

---
## Public API
| Method | Description |
|--------|-------------|
| `void setFrameRate(int fps)` | Manually change the target FPS. Values ≤ 0 are ignored. |
| `void update()` | Main loop run on its own thread. You normally do **not** call this directly – see `main.cpp`. |

---
## Event Reference
| Event | Payload |
|-------|---------|
| `TimeElapsedEvent` | `long long timeElapsed` – real-time passed since last tick (ms) |
| `DrawEvent` | `long long timeElapsed` – same value, supplied for convenience |
| `SetFrameRateEvent` | `int frameRate` – new FPS target |
| `StopEvent` | *none* |

---
## Implementation notes
1. The timer uses `usleep()` for sub-millisecond precision when it is running ahead of schedule.
2. When the thread falls behind (e.g. the CPU is busy) it immediately emits the events and logs the delay so that subsystems can adapt.
3. The event-driven design keeps the timer decoupled from concrete subsystems and makes unit-testing straightforward.

---
Made with ❤️ by the MeetingBox-CPP team.
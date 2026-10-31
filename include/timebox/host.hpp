#ifndef TIMEBOX_HOST_HPP
#define TIMEBOX_HOST_HPP

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

// Latest-frame slots for a dumb box. One present thread copies the focused
// slot and draws it. Control calls only write slots; they do not wait for SDL.
// A missing or late frame keeps the pixels already on the LEDs.

struct TimeboxProcess {
  int id = 0;
  std::string name;
  std::string kind;
  bool focused = false;
  int seconds = 0;
  int remaining_ms = 0;
  uint64_t frame_seq = 0;
  bool done = false;
};

struct TimeboxStatus {
  int focused_id = 0;
  std::string focused_name;
  int process_count = 0;
  int capacity = 0;
  uint64_t present_count = 0;
  bool present_running = false;
  bool sdl_ok = false;
};

class TimeboxHost {
 public:
  static constexpr int kCapacity = 8;
  static constexpr int kWidth = 320;
  static constexpr int kHeight = 64;
  static constexpr int kFrameBytes = kWidth * kHeight * 3;
  // Same 20 ms step as the timer demo's InterruptTimer (50 Hz).
  static constexpr int kPresentPeriodMs = 20;
  static constexpr int kDefaultTimerSeconds = 60;

  TimeboxHost();
  ~TimeboxHost();

  TimeboxHost(const TimeboxHost &) = delete;
  TimeboxHost &operator=(const TimeboxHost &) = delete;

  // present_loop draws the focused slot. timer_loop ticks InterruptTimer and
  // repaints every timer into its own slot, focused or not.
  void start(bool present_loop = true, bool timer_loop = true);
  void shutdown();

  // seconds < 0 uses kDefaultTimerSeconds for a timer and is ignored for paint.
  // focus switches the LEDs and does not stop any other process.
  std::string start_process(const std::string &kind, const std::string &name,
                            int seconds, bool focus, TimeboxProcess *created);
  std::string stop_process(int id, const std::string &name, TimeboxProcess *stopped);
  std::string show_process(int id, const std::string &name, TimeboxProcess *focused);
  // rgb is packed RGB888, kWidth * kHeight, no padding. Paint slots only.
  std::string paint_frame(int id, const uint8_t *rgb, size_t nbytes,
                          TimeboxProcess *updated);
  // id 0 and an empty name read the focused process.
  std::string snapshot(int id, const std::string &name, std::vector<uint8_t> *frame,
                       TimeboxProcess *info);

  std::vector<TimeboxProcess> list_processes() const;
  TimeboxStatus status() const;

  // Test clock. The live loop uses InterruptTimer::tick instead.
  void advance_timers(long long dt_ms);

  uint64_t present_count() const;
  std::vector<uint8_t> presented_frame() const;
  bool wait_present_count(uint64_t target, int timeout_ms) const;

  // Sleep inside the present thread after it has released the slot mutex.
  // host_test uses this to show paint_frame does not wait on present.
  void set_present_stall_ms(int ms);
  bool wait_until_present_busy(int timeout_ms) const;

 private:
  enum class Kind { Timer, Paint };

  struct Slot {
    bool alive = false;
    int id = 0;
    std::string name;
    Kind kind = Kind::Paint;
    int total_ms = 0;
    int remaining_ms = 0;
    std::vector<uint8_t> frame;
    uint64_t frame_seq = 0;
  };

  TimeboxProcess view_locked(const Slot &slot) const;
  int index_locked(int id, const std::string &name, std::string &error) const;
  void on_timer_draw(long long dt_ms);
  void present_loop();
  void timer_loop();
  bool wait_stop(int ms, const std::atomic<bool> &flag);

  mutable std::mutex mu_;
  Slot slots_[kCapacity];
  int next_id_ = 1;
  int focus_id_ = 0;
  bool loops_started_ = false;

  std::atomic<bool> stop_present_{false};
  std::atomic<bool> stop_timers_{false};
  std::mutex stop_mu_;
  std::condition_variable stop_cv_;

  mutable std::mutex present_mu_;
  mutable std::condition_variable present_cv_;
  std::vector<uint8_t> presented_;
  uint64_t present_count_ = 0;
  std::atomic<bool> present_running_{false};
  std::atomic<bool> sdl_ok_{false};
  std::atomic<int> present_stall_ms_{0};
  std::atomic<bool> present_busy_{false};

  std::thread present_thread_;
  std::thread timer_thread_;
};

#endif

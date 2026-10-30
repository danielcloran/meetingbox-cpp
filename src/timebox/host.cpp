#include "timebox/host.hpp"

#include "events/event_queue.hpp"
#include "events/events.hpp"
#include "graphics/graphics.hpp"
#include "graphics/timebox_timer.hpp"
#include "utils/interrupt_timer.hpp"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <utility>

namespace {

constexpr int kMaxTimerSeconds = 24 * 3600;

static_assert(TimeboxHost::kWidth == LED_WIDTH, "strip width");
static_assert(TimeboxHost::kHeight == LED_HEIGHT, "strip height");

bool valid_name(const std::string &name, std::string &error) {
  if (name.empty() || name.size() > 64) {
    error = "name must be 1..64 characters";
    return false;
  }
  if (name.find('\n') != std::string::npos || name.find('\r') != std::string::npos) {
    error = "name must be one line";
    return false;
  }
  return true;
}

void render_timer_rgb(std::vector<uint8_t> &frame, int remaining_ms, int total_ms) {
  graphics::GraphicsContext ctx(LED_WIDTH, LED_HEIGHT);
  graphics::paint_timebox_timer(ctx, remaining_ms, total_ms);
  const auto &px = ctx.get_pixels();
  frame.resize(static_cast<size_t>(TimeboxHost::kFrameBytes));
  for (size_t i = 0; i < px.size(); ++i) {
    frame[i * 3] = px[i].r;
    frame[i * 3 + 1] = px[i].g;
    frame[i * 3 + 2] = px[i].b;
  }
}

void draw_rgb(graphics::SDL2Renderer &renderer, const std::vector<uint8_t> &rgb) {
  graphics::GraphicsContext ctx(LED_WIDTH, LED_HEIGHT);
  const int n = TimeboxHost::kWidth * TimeboxHost::kHeight;
  for (int i = 0; i < n; ++i) {
    const uint8_t *p = rgb.data() + static_cast<size_t>(i) * 3;
    ctx.set_pixel(i % TimeboxHost::kWidth, i / TimeboxHost::kWidth,
                  graphics::Pixel(p[0], p[1], p[2]));
  }
  renderer.draw(ctx);
  renderer.should_close();
}

bool kind_from(const std::string &text, int *kind_timer) {
  if (text == "timer") {
    *kind_timer = 1;
    return true;
  }
  if (text == "paint") {
    *kind_timer = 0;
    return true;
  }
  return false;
}

}  // namespace

TimeboxHost::TimeboxHost() = default;

TimeboxHost::~TimeboxHost() { shutdown(); }

void TimeboxHost::start(bool present_loop_enabled, bool timer_loop_enabled) {
  if (loops_started_)
    return;
  loops_started_ = true;
  if (!::getenv("SDL_VIDEODRIVER"))
    ::setenv("SDL_VIDEODRIVER", "dummy", 0);
  if (present_loop_enabled) {
    present_running_.store(true);
    present_thread_ = std::thread([this] { present_loop(); });
  }
  if (timer_loop_enabled)
    timer_thread_ = std::thread([this] { timer_loop(); });
}

void TimeboxHost::shutdown() {
  {
    std::lock_guard<std::mutex> lock(stop_mu_);
    stop_present_.store(true);
    stop_timers_.store(true);
  }
  stop_cv_.notify_all();
  if (present_thread_.joinable())
    present_thread_.join();
  if (timer_thread_.joinable())
    timer_thread_.join();
  present_running_.store(false);
}

bool TimeboxHost::wait_stop(int ms, const std::atomic<bool> &flag) {
  std::unique_lock<std::mutex> lock(stop_mu_);
  stop_cv_.wait_for(lock, std::chrono::milliseconds(ms), [&] { return flag.load(); });
  return flag.load();
}

TimeboxProcess TimeboxHost::view_locked(const Slot &slot) const {
  TimeboxProcess view;
  view.id = slot.id;
  view.name = slot.name;
  view.kind = slot.kind == Kind::Timer ? "timer" : "paint";
  view.focused = slot.id == focus_id_ && focus_id_ != 0;
  view.seconds = slot.total_ms / 1000;
  view.remaining_ms = slot.remaining_ms;
  view.frame_seq = slot.frame_seq;
  view.done = slot.kind == Kind::Timer && slot.total_ms > 0 && slot.remaining_ms <= 0;
  return view;
}

int TimeboxHost::index_locked(int id, const std::string &name, std::string &error) const {
  if (id != 0) {
    for (int i = 0; i < kCapacity; ++i) {
      if (slots_[i].alive && slots_[i].id == id)
        return i;
    }
    error = "no such process";
    return -1;
  }
  if (!name.empty()) {
    for (int i = 0; i < kCapacity; ++i) {
      if (slots_[i].alive && slots_[i].name == name)
        return i;
    }
    error = "no such process";
    return -1;
  }
  error = "id or name required";
  return -1;
}

std::string TimeboxHost::start_process(const std::string &kind, const std::string &name,
                                       int seconds, bool focus, TimeboxProcess *created) {
  int is_timer = 0;
  if (!kind_from(kind, &is_timer))
    return "kind must be timer or paint";
  std::string error;
  if (!valid_name(name, error))
    return error;

  const Kind slot_kind = is_timer ? Kind::Timer : Kind::Paint;
  int total_ms = 0;
  if (slot_kind == Kind::Timer) {
    const int sec = seconds < 0 ? kDefaultTimerSeconds : seconds;
    if (sec <= 0 || sec > kMaxTimerSeconds)
      return "seconds must be 1..86400";
    total_ms = sec * 1000;
  }

  std::vector<uint8_t> frame(static_cast<size_t>(kFrameBytes), 0);
  if (slot_kind == Kind::Timer)
    render_timer_rgb(frame, total_ms, total_ms);

  int started_id = 0;
  bool started_focused = false;
  {
    std::lock_guard<std::mutex> lock(mu_);
    for (const auto &slot : slots_) {
      if (slot.alive && slot.name == name)
        return "name already in use";
    }
    int index = -1;
    for (int i = 0; i < kCapacity; ++i) {
      if (!slots_[i].alive) {
        index = i;
        break;
      }
    }
    if (index < 0)
      return "process table full (8)";

    Slot &slot = slots_[index];
    slot = Slot{};
    slot.alive = true;
    slot.id = next_id_++;
    slot.name = name;
    slot.kind = slot_kind;
    slot.total_ms = total_ms;
    slot.remaining_ms = total_ms;
    slot.frame = std::move(frame);
    slot.frame_seq = 1;
    if (focus)
      focus_id_ = slot.id;
    started_id = slot.id;
    started_focused = slot.id == focus_id_;
    if (created)
      *created = view_locked(slot);
  }
  std::cerr << "timebox: start " << kind << " " << name << " id=" << started_id
            << (started_focused ? " focused" : "") << "\n";
  return {};
}

std::string TimeboxHost::stop_process(int id, const std::string &name, TimeboxProcess *stopped) {
  std::string stopped_name;
  int stopped_id = 0;
  {
    std::lock_guard<std::mutex> lock(mu_);
    std::string error;
    const int index = index_locked(id, name, error);
    if (index < 0)
      return error;
    if (focus_id_ == slots_[index].id)
      focus_id_ = 0;
    if (stopped)
      *stopped = view_locked(slots_[index]);
    stopped_name = slots_[index].name;
    stopped_id = slots_[index].id;
    slots_[index] = Slot{};
  }
  std::cerr << "timebox: stop " << stopped_name << " id=" << stopped_id << "\n";
  return {};
}

std::string TimeboxHost::show_process(int id, const std::string &name, TimeboxProcess *focused) {
  std::string shown_name;
  int shown_id = 0;
  {
    std::lock_guard<std::mutex> lock(mu_);
    std::string error;
    const int index = index_locked(id, name, error);
    if (index < 0)
      return error;
    focus_id_ = slots_[index].id;
    if (focused)
      *focused = view_locked(slots_[index]);
    shown_name = slots_[index].name;
    shown_id = slots_[index].id;
  }
  std::cerr << "timebox: show " << shown_name << " id=" << shown_id << "\n";
  return {};
}

std::string TimeboxHost::paint_frame(int id, const uint8_t *rgb, size_t nbytes,
                                     TimeboxProcess *updated) {
  if (nbytes != static_cast<size_t>(kFrameBytes) || rgb == nullptr)
    return "frame must be 320x64 RGB888 (61440 bytes)";
  std::lock_guard<std::mutex> lock(mu_);
  std::string error;
  const int index = index_locked(id, "", error);
  if (index < 0)
    return error;
  Slot &slot = slots_[index];
  if (slot.kind != Kind::Paint)
    return "process is not a paint process";
  if (slot.frame.size() != static_cast<size_t>(kFrameBytes))
    slot.frame.assign(static_cast<size_t>(kFrameBytes), 0);
  std::memcpy(slot.frame.data(), rgb, static_cast<size_t>(kFrameBytes));
  slot.frame_seq++;
  if (updated)
    *updated = view_locked(slot);
  return {};
}

std::string TimeboxHost::snapshot(int id, const std::string &name, std::vector<uint8_t> *frame,
                                  TimeboxProcess *info) {
  std::lock_guard<std::mutex> lock(mu_);
  int use_id = id;
  std::string use_name = name;
  if (use_id == 0 && use_name.empty()) {
    if (focus_id_ == 0)
      return "no focused process";
    use_id = focus_id_;
  }
  std::string error;
  const int index = index_locked(use_id, use_name, error);
  if (index < 0)
    return error;
  if (frame)
    *frame = slots_[index].frame;
  if (info)
    *info = view_locked(slots_[index]);
  return {};
}

std::vector<TimeboxProcess> TimeboxHost::list_processes() const {
  std::lock_guard<std::mutex> lock(mu_);
  std::vector<TimeboxProcess> out;
  for (const auto &slot : slots_) {
    if (slot.alive)
      out.push_back(view_locked(slot));
  }
  return out;
}

TimeboxStatus TimeboxHost::status() const {
  TimeboxStatus status;
  status.capacity = kCapacity;
  {
    std::lock_guard<std::mutex> lock(mu_);
    status.focused_id = focus_id_;
    for (const auto &slot : slots_) {
      if (!slot.alive)
        continue;
      ++status.process_count;
      if (slot.id == focus_id_)
        status.focused_name = slot.name;
    }
  }
  {
    std::lock_guard<std::mutex> lock(present_mu_);
    status.present_count = present_count_;
  }
  status.present_running = present_running_.load();
  status.sdl_ok = sdl_ok_.load();
  return status;
}

void TimeboxHost::on_timer_draw(long long dt_ms) {
  if (dt_ms < 0)
    dt_ms = 0;
  struct Job {
    int index;
    int id;
    int remaining;
    int total;
  };
  std::vector<Job> jobs;
  {
    std::lock_guard<std::mutex> lock(mu_);
    for (int i = 0; i < kCapacity; ++i) {
      Slot &slot = slots_[i];
      if (!slot.alive || slot.kind != Kind::Timer)
        continue;
      if (slot.total_ms > 0 && dt_ms > 0 && slot.remaining_ms > 0) {
        if (dt_ms >= slot.remaining_ms)
          slot.remaining_ms = 0;
        else
          slot.remaining_ms -= static_cast<int>(dt_ms);
      }
      jobs.push_back(Job{i, slot.id, slot.remaining_ms, slot.total_ms});
    }
  }
  for (const Job &job : jobs) {
    std::vector<uint8_t> frame;
    render_timer_rgb(frame, job.remaining, job.total);
    std::lock_guard<std::mutex> lock(mu_);
    Slot &slot = slots_[job.index];
    if (!slot.alive || slot.id != job.id || slot.kind != Kind::Timer)
      continue;
    slot.frame = std::move(frame);
    slot.frame_seq++;
  }
}

void TimeboxHost::advance_timers(long long dt_ms) { on_timer_draw(dt_ms); }

void TimeboxHost::present_loop() {
  if (!::getenv("SDL_VIDEODRIVER"))
    ::setenv("SDL_VIDEODRIVER", "dummy", 0);
  graphics::SDL2Renderer renderer;
  const bool sdl_ok = renderer.init_capture();
  sdl_ok_.store(sdl_ok);
  if (!sdl_ok) {
    std::cerr << "timebox: SDL headless init failed; holding frames in memory\n";
  } else {
    std::cerr << "timebox: present loop " << kPresentPeriodMs << " ms\n";
  }

  std::vector<uint8_t> held(static_cast<size_t>(kFrameBytes), 0);
  while (!stop_present_.load()) {
    {
      std::lock_guard<std::mutex> lock(mu_);
      for (const auto &slot : slots_) {
        if (!slot.alive || slot.id != focus_id_)
          continue;
        if (slot.frame.size() == held.size())
          std::copy(slot.frame.begin(), slot.frame.end(), held.begin());
        break;
      }
    }

    // Slot mutex is released. A late client does not stall this draw, and a
    // slow draw does not stall paint_frame. No new frame: keep `held`.
    present_busy_.store(true);
    const int stall = present_stall_ms_.load();
    if (stall > 0 && wait_stop(stall, stop_present_)) {
      present_busy_.store(false);
      break;
    }
    if (stop_present_.load()) {
      present_busy_.store(false);
      break;
    }
    if (sdl_ok)
      draw_rgb(renderer, held);
    present_busy_.store(false);

    {
      std::lock_guard<std::mutex> lock(present_mu_);
      presented_ = held;
      ++present_count_;
    }
    present_cv_.notify_all();

    if (wait_stop(kPresentPeriodMs, stop_present_))
      break;
  }
  present_running_.store(false);
}

void TimeboxHost::timer_loop() {
  // InterruptTimer::tick publishes the draw event. update() would block in its
  // own sleep and only stop via the event queue, so the host ticks it.
  InterruptTimer clock;
  clock.setFrameRate(1000 / kPresentPeriodMs);
  events::ScopedRemover listeners(events::queue);
  listeners.appendListener(EventType::draw, [this](events::EventPointer ev) {
    const auto *draw = static_cast<const DrawEvent *>(ev.get());
    on_timer_draw(draw->getTimeElapsed());
  });
  while (!stop_timers_.load()) {
    clock.tick(kPresentPeriodMs);
    events::queue.process();
    if (wait_stop(kPresentPeriodMs, stop_timers_))
      break;
  }
}

uint64_t TimeboxHost::present_count() const {
  std::lock_guard<std::mutex> lock(present_mu_);
  return present_count_;
}

std::vector<uint8_t> TimeboxHost::presented_frame() const {
  std::lock_guard<std::mutex> lock(present_mu_);
  return presented_;
}

bool TimeboxHost::wait_present_count(uint64_t target, int timeout_ms) const {
  std::unique_lock<std::mutex> lock(present_mu_);
  return present_cv_.wait_for(lock, std::chrono::milliseconds(timeout_ms),
                              [&] { return present_count_ >= target; });
}

void TimeboxHost::set_present_stall_ms(int ms) {
  present_stall_ms_.store(ms < 0 ? 0 : ms);
}

bool TimeboxHost::wait_until_present_busy(int timeout_ms) const {
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
  while (std::chrono::steady_clock::now() < deadline) {
    if (present_busy_.load())
      return true;
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
  return present_busy_.load();
}

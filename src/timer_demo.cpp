#include "demo/timer_demo.hpp"

#include "src/gif_writer.hpp"
#include "graphics/graphics.hpp"
#include "graphics/sdl_renderer.hpp"
#include "graphics/timebox_timer.hpp"
#include "processes/timer.hpp"
#include "utils/interrupt_timer.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "json/json.h"

namespace {

std::atomic<bool> g_stop{false};

void on_sigint(int) { g_stop.store(true); }

struct Options {
  bool headless = false;
  bool headless_set = false;
  bool realtime = false;
  bool benchmark = false;
  int seconds = 0;
  int step_ms = 100;
  bool step_set = false;
  std::string gif_path;
  std::string ppm_dir;
};

void print_usage() {
  std::cout
      << "Usage: timer_demo [--demo=timer] [--no-audio] [--headless] [--realtime]\n"
      << "                  [--seconds=N] [--step-ms=N] [--gif=path] [--ppm-dir=path]\n"
      << "\n"
      << "  Wall faces use the original TimeBox blue row-fade. The lid is the\n"
      << "  middle strip panel (center of the cross) and drains in a spiral\n"
      << "  toward the middle. InterruptTimer is the clock. Audio is off.\n"
      << "\n"
      << "  --headless   SDL software renderer, no window. This is the default\n"
      << "               when DISPLAY is unset, or when --gif / --ppm-dir is set.\n"
      << "  --realtime   Live present loop (also the default GIF). InterruptTimer\n"
      << "               ticks every 20 ms and SDL presents on that same cadence.\n"
      << "  --seconds=N  Meeting length in seconds (headless default 8, window 60).\n"
      << "  --step-ms=N  Stepped export only, not the live rate. One frame per step.\n"
      << "  --benchmark  Time paint+present: tight, legacy 16ms sleep, and 20ms cadence.\n"
      << "  --gif=PATH   Write a GIF of each live present (50 Hz, 20 ms per frame).\n"
      << "  --ppm-dir=P  Also write one 192x192 PPM per recorded frame.\n";
}

bool take_value(int &i, int argc, char **argv, const std::string &flag,
                std::string &value, std::string &error) {
  const std::string prefix = flag + "=";
  const std::string arg = argv[i];
  if (arg.rfind(prefix, 0) == 0) {
    value = arg.substr(prefix.size());
  } else if (arg == flag) {
    if (i + 1 >= argc) {
      error = flag + " requires a value";
      return false;
    }
    value = argv[++i];
  } else {
    return false;
  }
  if (value.empty()) {
    error = flag + " requires a value";
    return false;
  }
  return true;
}

bool parse_int(const std::string &text, int &out) {
  if (text.empty())
    return false;
  char *end = nullptr;
  const long value = std::strtol(text.c_str(), &end, 10);
  if (end == text.c_str() || *end != '\0' || value <= 0 || value > 24 * 3600)
    return false;
  out = static_cast<int>(value);
  return true;
}

bool parse_options(int argc, char **argv, Options &opt, bool &help,
                   std::string &error) {
  help = false;
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    std::string value;
    if (arg == "--help" || arg == "-h") {
      help = true;
      return true;
    }
    if (arg == "--demo=timer" || arg == "--no-audio")
      continue;
    if (arg == "--headless") {
      opt.headless = true;
      opt.headless_set = true;
      continue;
    }
    if (arg == "--realtime") {
      opt.realtime = true;
      continue;
    }
    if (arg == "--benchmark") {
      opt.benchmark = true;
      continue;
    }
    if (arg.rfind("--seconds", 0) == 0) {
      if (!take_value(i, argc, argv, "--seconds", value, error) ||
          !parse_int(value, opt.seconds)) {
        if (error.empty())
          error = "--seconds expects a positive number of seconds";
        return false;
      }
      continue;
    }
    if (arg.rfind("--step-ms", 0) == 0) {
      if (!take_value(i, argc, argv, "--step-ms", value, error) ||
          !parse_int(value, opt.step_ms)) {
        if (error.empty())
          error = "--step-ms expects a positive number of milliseconds";
        return false;
      }
      opt.step_set = true;
      continue;
    }
    if (arg.rfind("--gif", 0) == 0) {
      if (!take_value(i, argc, argv, "--gif", opt.gif_path, error))
        return false;
      continue;
    }
    if (arg.rfind("--ppm-dir", 0) == 0) {
      if (!take_value(i, argc, argv, "--ppm-dir", opt.ppm_dir, error))
        return false;
      continue;
    }
    if (arg.rfind("--demo=", 0) == 0) {
      error = "only --demo=timer is supported";
      return false;
    }
    error = "unknown argument: " + arg;
    return false;
  }
  return true;
}

bool ensure_parent(const std::filesystem::path &path, std::string &error) {
  const auto parent = path.parent_path();
  if (parent.empty())
    return true;
  std::error_code ec;
  std::filesystem::create_directories(parent, ec);
  if (ec) {
    error = "could not create " + parent.string() + ": " + ec.message();
    return false;
  }
  return true;
}

bool write_frames(const Options &opt,
                  const std::vector<graphics::GraphicsContext> &frames,
                  int delay_cs) {
  if (!opt.ppm_dir.empty()) {
    std::string error;
    if (!ensure_parent(std::filesystem::path(opt.ppm_dir) / "frame.ppm", error)) {
      std::cerr << error << "\n";
      return false;
    }
    for (size_t i = 0; i < frames.size(); ++i) {
      char name[32];
      std::snprintf(name, sizeof(name), "frame_%03zu.ppm", i);
      const auto path = std::filesystem::path(opt.ppm_dir) / name;
      if (!frames[i].save_to_ppm(path.string())) {
        std::cerr << "could not write " << path << "\n";
        return false;
      }
    }
    std::cout << "Wrote " << frames.size() << " PPM frames in " << opt.ppm_dir
              << "\n";
  }
  if (!opt.gif_path.empty()) {
    std::string error;
    if (!ensure_parent(opt.gif_path, error)) {
      std::cerr << error << "\n";
      return false;
    }
    if (!write_animated_gif(opt.gif_path, frames, delay_cs, 80, 3, error)) {
      std::cerr << error << "\n";
      return false;
    }
    std::cout << "Wrote " << std::filesystem::absolute(opt.gif_path).string()
              << " (" << frames.size() << " frames, 576x576)\n";
  }
  return true;
}

// Times the current SDL present path. Tight = back-to-back draw+present.
// Paced = the windowed loop's extra 16ms sleep after each present.
int benchmark_present(int seconds) {
  setenv("SDL_VIDEODRIVER", "dummy", 1);
  graphics::SDL2Renderer renderer;
  if (!renderer.init_capture()) {
    std::cerr << "SDL headless init failed: " << SDL_GetError() << "\n";
    return 1;
  }
  graphics::GraphicsContext strip(LED_WIDTH, LED_HEIGHT);
  graphics::paint_timebox_timer(strip, 4000, 8000);

  auto run = [&](bool paced, const char *label) {
    const auto start = std::chrono::steady_clock::now();
    const auto deadline = start + std::chrono::seconds(seconds);
    int frames = 0;
    double paint_ms = 0;
    while (std::chrono::steady_clock::now() < deadline) {
      const auto t0 = std::chrono::steady_clock::now();
      renderer.draw(strip);
      const auto t1 = std::chrono::steady_clock::now();
      paint_ms += std::chrono::duration<double, std::milli>(t1 - t0).count();
      ++frames;
      if (paced) {
        const auto wake = t1 + std::chrono::milliseconds(16);
        if (wake > std::chrono::steady_clock::now())
          std::this_thread::sleep_until(wake);
      }
    }
    const double elapsed =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    std::cout << "present " << label << ": " << frames << " frames in " << elapsed
              << "s = " << (frames / elapsed) << " Hz, paint+present avg "
              << (paint_ms / frames) << " ms\n";
  };
  run(false, "tight");
  run(true, "legacy-sleep-16ms-after");

  const auto start = std::chrono::steady_clock::now();
  const auto deadline = start + std::chrono::seconds(seconds);
  int frames = 0;
  double paint_ms = 0;
  constexpr int kPeriodMs = 20;
  while (std::chrono::steady_clock::now() < deadline) {
    const auto target = start + std::chrono::milliseconds(static_cast<long long>(frames + 1) * kPeriodMs);
    const auto t0 = std::chrono::steady_clock::now();
    renderer.draw(strip);
    const auto t1 = std::chrono::steady_clock::now();
    paint_ms += std::chrono::duration<double, std::milli>(t1 - t0).count();
    ++frames;
    if (t1 < target)
      std::this_thread::sleep_until(target);
  }
  const double elapsed =
      std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
  std::cout << "present cadence-20ms: " << frames << " frames in " << elapsed
            << "s = " << (frames / elapsed) << " Hz, paint+present avg "
            << (paint_ms / frames) << " ms\n";
  return 0;
}

void log_remaining(const TimerProcess &timer) {
  const int remaining = timer.remaining_ms();
  const int seconds = (remaining + 999) / 1000;
  std::cout << "remaining " << seconds << "s (" << remaining << " ms)\n";
}

int run_headless(const Options &opt) {
  const int total_ms = opt.seconds * 1000;
  const int frame_budget = total_ms / opt.step_ms + 1;
  if (frame_budget > 400) {
    std::cerr << "Refusing to capture " << frame_budget
              << " frames. Increase --step-ms or lower --seconds.\n";
    return 2;
  }

  setenv("SDL_VIDEODRIVER", "dummy", 1);
  graphics::SDL2Renderer renderer;
  if (!renderer.init_capture()) {
    std::cerr << "SDL headless init failed: " << SDL_GetError() << "\n";
    return 1;
  }

  graphics::context_ =
      std::make_unique<graphics::GraphicsContext>(LED_WIDTH, LED_HEIGHT);
  Json::Value info;
  info["name"] = "Meeting";
  info["time"] = opt.seconds;
  TimerProcess timer(1, 0, info);
  if (timer.total_ms() <= 0) {
    std::cerr << "Timer duration is 0\n";
    return 1;
  }
  InterruptTimer clock;
  clock.setFrameRate(30);

  std::vector<graphics::GraphicsContext> frames;
  frames.reserve(static_cast<size_t>(frame_budget));
  int logged_second = -1;
  auto capture = [&]() -> bool {
    events::queue.process();
    graphics::GraphicsContext box(graphics::kBoxWidth, graphics::kBoxHeight);
    std::string error;
    if (!renderer.capture(*graphics::context_, box, error)) {
      std::cerr << error << "\n";
      return false;
    }
    const int second = (timer.remaining_ms() + 999) / 1000;
    if (second != logged_second) {
      log_remaining(timer);
      logged_second = second;
    }
    frames.push_back(std::move(box));
    return true;
  };

  std::thread ticker;
  if (opt.realtime) {
    clock.tick(0);
    if (!capture())
      return 1;
    ticker = std::thread([&clock] { clock.update(); });
    const auto started = std::chrono::steady_clock::now();
    auto next = started + std::chrono::milliseconds(opt.step_ms);
    while (!g_stop.load() && timer.remaining_ms() > 0) {
      events::queue.process();
      const auto now = std::chrono::steady_clock::now();
      if (now >= next) {
        if (!capture()) {
          g_stop.store(true);
          break;
        }
        next += std::chrono::milliseconds(opt.step_ms);
      }
      if (now - started > std::chrono::seconds(opt.seconds + 2))
        break;
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    events::queue.enqueue(std::make_shared<StopEvent>());
    events::queue.process();
    if (ticker.joinable())
      ticker.join();
    events::queue.process();
    if (timer.remaining_ms() == 0)
      capture();
  } else {
    clock.tick(0);
    if (!capture())
      return 1;
    int elapsed = 0;
    while (elapsed < total_ms && !g_stop.load()) {
      const int step = std::min(opt.step_ms, total_ms - elapsed);
      clock.tick(step);
      if (!capture())
        return 1;
      elapsed += step;
    }
  }

  if (frames.size() < 2) {
    std::cerr << "timer demo captured fewer than 2 frames\n";
    return 1;
  }
  const int delay_cs = std::max(2, opt.step_ms / 10);
  if (!write_frames(opt, frames, delay_cs))
    return 1;
  std::cout << "Headless TimeBox countdown finished (" << frames.size()
            << " frames).\n";
  return 0;
}

// Live present period. GIF delays are integer centiseconds, so 20 ms (50 Hz)
// is the 16 ms windowed target expressed as a rate the file can play back 1:1.
// setFrameRate(50) is 1000/50 = 20 ms, the same tick.
constexpr int kLivePeriodMs = 20;
constexpr int kLiveHz = 1000 / kLivePeriodMs;

int run_live(const Options &opt) {
  const bool record = !opt.gif_path.empty() || !opt.ppm_dir.empty();
  graphics::SDL2Renderer headless_renderer;
  graphics::SDL2Renderer *sdl = nullptr;
  if (opt.headless) {
    setenv("SDL_VIDEODRIVER", "dummy", 1);
    if (!headless_renderer.init_capture()) {
      std::cerr << "SDL headless init failed: " << SDL_GetError() << "\n";
      return 1;
    }
    sdl = &headless_renderer;
    graphics::context_ =
        std::make_unique<graphics::GraphicsContext>(LED_WIDTH, LED_HEIGHT);
  } else {
    graphics::initialize();
    sdl = dynamic_cast<graphics::SDL2Renderer *>(graphics::renderer_.get());
    if (!sdl || !sdl->ready()) {
      std::cerr << "SDL window failed to open. Re-run with --headless"
                << " (no display required).\n";
      graphics::quit();
      return 1;
    }
    std::cout << "TimeBox countdown on the LED faces. Close the window or press Ctrl+C.\n";
  }

  Json::Value info;
  info["name"] = "Meeting";
  info["time"] = opt.seconds;
  TimerProcess timer(1, 0, info);
  if (timer.total_ms() <= 0) {
    std::cerr << "Timer duration is 0\n";
    if (!opt.headless)
      graphics::quit();
    return 1;
  }

  const int frame_cap = opt.seconds * kLiveHz + 8;
  if (record && frame_cap > 800) {
    std::cerr << "Refusing to record about " << frame_cap
              << " live frames. Lower --seconds.\n";
    if (!opt.headless)
      graphics::quit();
    return 2;
  }

  InterruptTimer clock;
  clock.setFrameRate(kLiveHz);
  std::vector<graphics::GraphicsContext> frames;
  if (record)
    frames.reserve(static_cast<size_t>(frame_cap));
  std::vector<std::chrono::steady_clock::time_point> stamps;
  stamps.reserve(static_cast<size_t>(frame_cap));
  double paint_ms = 0;
  int logged_second = -1;

  auto present = [&]() -> bool {
    events::queue.process();
    if (!graphics::context_)
      return false;
    const auto t0 = std::chrono::steady_clock::now();
    sdl->draw(*graphics::context_);
    const auto t1 = std::chrono::steady_clock::now();
    paint_ms += std::chrono::duration<double, std::milli>(t1 - t0).count();
    stamps.push_back(t1);
    if (record) {
      if (static_cast<int>(frames.size()) >= frame_cap)
        return false;
      graphics::GraphicsContext box(graphics::kBoxWidth, graphics::kBoxHeight);
      graphics::paint_box_layout(*graphics::context_, box);
      frames.push_back(std::move(box));
    }
    const int second = (timer.remaining_ms() + 999) / 1000;
    if (second != logged_second) {
      log_remaining(timer);
      logged_second = second;
    }
    return true;
  };

  clock.tick(0);
  if (!present())
    return 1;
  std::thread ticker([&clock] { clock.update(); });
  const auto origin = stamps.front();
  int periods = 1;
  while (!g_stop.load() && timer.remaining_ms() > 0 && !sdl->should_close()) {
    const auto target = origin + std::chrono::milliseconds(
                                     static_cast<long long>(periods) * kLivePeriodMs);
    const auto now = std::chrono::steady_clock::now();
    if (now < target)
      std::this_thread::sleep_until(target);
    if (!present())
      break;
    ++periods;
    if (std::chrono::steady_clock::now() - origin >
        std::chrono::seconds(opt.seconds + 2))
      break;
  }

  events::queue.enqueue(std::make_shared<StopEvent>());
  events::queue.process();
  if (ticker.joinable())
    ticker.join();
  events::queue.process();

  double hz = 0;
  double span = 0;
  if (stamps.size() >= 2) {
    span = std::chrono::duration<double>(stamps.back() - stamps.front()).count();
    hz = static_cast<double>(stamps.size() - 1) / span;
  }
  const double paint_avg = stamps.empty() ? 0 : paint_ms / static_cast<double>(stamps.size());
  std::cout << "live present: " << stamps.size() << " frames in " << span
            << "s = " << hz << " Hz, paint+present avg " << paint_avg
            << " ms, cadence " << kLivePeriodMs << " ms\n";

  int status = 0;
  if (record) {
    if (frames.size() < 2) {
      std::cerr << "timer demo recorded fewer than 2 frames\n";
      status = 1;
    } else if (!write_frames(opt, frames, kLivePeriodMs / 10)) {
      status = 1;
    } else {
      std::cout << "GIF playback " << (100 / (kLivePeriodMs / 10))
                << " Hz (delay " << (kLivePeriodMs / 10) << " cs), matching the live cadence.\n";
    }
  }

  if (!opt.headless)
    graphics::quit();
  if (!opt.headless || !record)
    std::cout << "Timer demo stopped.\n";
  return status;
}

}  // namespace

int run_timer_demo(int argc, char **argv) {
  Options opt;
  bool help = false;
  std::string error;
  if (!parse_options(argc, argv, opt, help, error)) {
    std::cerr << error << "\n";
    print_usage();
    return 2;
  }
  if (help) {
    print_usage();
    return 0;
  }

  if (!opt.headless_set) {
    const bool display =
        std::getenv("DISPLAY") != nullptr || std::getenv("WAYLAND_DISPLAY") != nullptr;
    opt.headless = !display;
  }
  if (!opt.gif_path.empty() || !opt.ppm_dir.empty())
    opt.headless = true;
  if (opt.seconds <= 0)
    opt.seconds = opt.headless ? 8 : 60;
  if (opt.headless && opt.gif_path.empty() && opt.ppm_dir.empty())
    opt.gif_path = "meeting_timer.gif";

  std::signal(SIGINT, on_sigint);
  if (opt.benchmark) {
    const int bench_seconds = opt.seconds > 0 ? std::min(opt.seconds, 5) : 2;
    return benchmark_present(bench_seconds);
  }
  std::cout << "TimeBox countdown, " << opt.seconds
            << "s. Audio and hotword are off.\n";

  // --step-ms is the old subsampled export. The default path presents live.
  if (opt.headless && opt.step_set)
    return run_headless(opt);
  opt.realtime = true;
  return run_live(opt);
}

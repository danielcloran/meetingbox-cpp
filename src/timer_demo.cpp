#include "demo/timer_demo.hpp"

#include "src/gif_writer.hpp"
#include "graphics/graphics.hpp"
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
  int seconds = 0;
  int step_ms = 100;
  std::string gif_path;
  std::string ppm_dir;
};

void print_usage() {
  std::cout
      << "Usage: timer_demo [--demo=timer] [--no-audio] [--headless] [--realtime]\n"
      << "                  [--seconds=N] [--step-ms=N] [--gif=path] [--ppm-dir=path]\n"
      << "\n"
      << "  Paints the original TimeBox countdown (blue row fade) on the five\n"
      << "  LED faces. InterruptTimer is the clock. Audio and hotword are not started.\n"
      << "\n"
      << "  --headless   SDL software renderer, no window. This is the default\n"
      << "               when DISPLAY is unset, or when --gif / --ppm-dir is set.\n"
      << "  --realtime   Drive the clock with InterruptTimer::update (wall clock).\n"
      << "               Headless without --realtime steps the same tick by --step-ms\n"
      << "               so the GIF is even and the command does not wait out the timer.\n"
      << "  --seconds=N  Meeting length in seconds (headless default 8, window 60).\n"
      << "  --step-ms=N  Headless frame step, or realtime capture interval (default 100).\n"
      << "  --gif=PATH   Write a looping GIF of the unfolded box (scaled 3x).\n"
      << "  --ppm-dir=P  Also write one 192x192 PPM per frame.\n";
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

int run_windowed(const Options &opt) {
  graphics::initialize();
  auto *sdl = dynamic_cast<graphics::SDL2Renderer *>(graphics::renderer_.get());
  if (!sdl || !sdl->ready()) {
    std::cerr << "SDL window failed to open. Re-run with --headless"
              << " (no display required).\n";
    graphics::quit();
    return 1;
  }

  Json::Value info;
  info["name"] = "Meeting";
  info["time"] = opt.seconds;
  TimerProcess timer(1, 0, info);
  if (timer.total_ms() <= 0) {
    std::cerr << "Timer duration is 0\n";
    graphics::quit();
    return 1;
  }

  InterruptTimer clock;
  clock.setFrameRate(30);
  clock.tick(0);
  events::queue.process();
  sdl->draw(*graphics::context_);

  std::thread ticker([&clock] { clock.update(); });
  int logged_second = -1;
  std::cout << "TimeBox countdown on the LED faces. Close the window or press Ctrl+C.\n";
  while (!g_stop.load() && !sdl->should_close()) {
    events::queue.process();
    if (!graphics::context_)
      break;
    sdl->draw(*graphics::context_);
    const int second = (timer.remaining_ms() + 999) / 1000;
    if (second != logged_second) {
      log_remaining(timer);
      logged_second = second;
    }
    if (timer.remaining_ms() <= 0)
      break;
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }

  events::queue.enqueue(std::make_shared<StopEvent>());
  events::queue.process();
  if (ticker.joinable())
    ticker.join();
  graphics::quit();
  std::cout << "Timer demo stopped.\n";
  return 0;
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
  std::cout << "TimeBox countdown, " << opt.seconds
            << "s. Audio and hotword are off.\n";

  if (opt.headless)
    return run_headless(opt);
  return run_windowed(opt);
}

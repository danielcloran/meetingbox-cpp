#include "events/event_queue.hpp"
#include "events/events.hpp"
#include "graphics/graphics.hpp"
#include "media/src/graphics/box_layout_spec.hpp"

#include <iostream>
#include <string>

namespace {

void print_usage() {
  std::cout
      << "Usage: graphics_test [--headless] [--ppm=path.ppm]\n"
      << "  --headless  Paint the unfolded box with the SDL software renderer\n"
      << "              (SDL_VIDEODRIVER=dummy) and exit. Non-zero on mismatch.\n"
      << "  --ppm=FILE  With --headless, write the painted box as a PPM image.\n";
}

}  // namespace

int main(int argc, char **argv) {
  bool headless = false;
  std::string ppm_path;

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--headless") {
      headless = true;
    } else if (arg == "--help" || arg == "-h") {
      print_usage();
      return 0;
    } else if (arg.rfind("--ppm=", 0) == 0) {
      ppm_path = arg.substr(6);
      if (ppm_path.empty()) {
        std::cerr << "--ppm requires a file path\n";
        return 2;
      }
    } else {
      std::cerr << "Unknown argument: " << arg << "\n";
      print_usage();
      return 2;
    }
  }

  if (headless) {
    std::cout << "Headless graphics test (box layout)..." << std::endl;
    const int status = graphics::testspec::run_box_paint_checks(ppm_path);
    if (status == 0) {
      std::cout << "Graphics Test Completed." << std::endl;
    }
    return status;
  }

  std::cout << "Starting Graphics Test (Box Layout)..." << std::endl;

  events::queue.appendListener(
      EventType::draw, [](const events::EventPointer &) {
        if (graphics::context_) {
          graphics::paint_graphics_test_pattern(*graphics::context_);
        }
      });

  events::queue.appendListener(
      EventType::stop,
      [](const events::EventPointer &) { graphics::quit(); });

  graphics::initialize();
  graphics::loop();

  std::cout << "Graphics Test Completed." << std::endl;
  return 0;
}

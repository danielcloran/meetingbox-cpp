#include "events/event_queue.hpp"
#include "events/events.hpp"
#include "graphics/graphics.hpp"
#include <chrono>
#include <iostream>
#include <thread>

int main() {
  std::cout << "Starting Graphics Test (Box Layout)..." << std::endl;

  // Register Draw Listener
  // Note: EventType and Event are in the global namespace based on events.hpp
  events::queue.appendListener(
      EventType::draw, [](const events::EventPointer &event) {
        if (graphics::context_) {
          graphics::context_->clear(graphics::Pixel(0, 0, 0));

          // Draw distinct colors for each panel to verify layout

          // Panel 0: Left (Red) -> Should appear at (0, 64)
          graphics::context_->draw_rectangle(0 * 64, 0, 64, 64,
                                             graphics::Pixel(255, 0, 0));
          graphics::context_->draw_text("LEFT", 0 * 64 + 10, 20,
                                        graphics::Pixel(255, 255, 255));

          // Panel 1: Front (Green) -> Should appear at (64, 64)
          graphics::context_->draw_rectangle(1 * 64, 0, 64, 64,
                                             graphics::Pixel(0, 255, 0));
          graphics::context_->draw_text("FRONT", 1 * 64 + 10, 20,
                                        graphics::Pixel(0, 0, 0));

          // Panel 2: Right (Blue) -> Should appear at (128, 64)
          graphics::context_->draw_rectangle(2 * 64, 0, 64, 64,
                                             graphics::Pixel(0, 0, 255));
          graphics::context_->draw_text("RIGHT", 2 * 64 + 10, 20,
                                        graphics::Pixel(255, 255, 255));

          // Panel 3: Back (Yellow) -> Should appear at (64, 128)
          graphics::context_->draw_rectangle(3 * 64, 0, 64, 64,
                                             graphics::Pixel(255, 255, 0));
          graphics::context_->draw_text("BACK", 3 * 64 + 10, 20,
                                        graphics::Pixel(0, 0, 0));

          // Panel 4: Top (Magenta) -> Should appear at (64, 0)
          graphics::context_->draw_rectangle(4 * 64, 0, 64, 64,
                                             graphics::Pixel(255, 0, 255));
          graphics::context_->draw_text("TOP", 4 * 64 + 10, 20,
                                        graphics::Pixel(255, 255, 255));
        }
      });

  // Register Stop Listener
  events::queue.appendListener(
      EventType::stop,
      [](const events::EventPointer &event) { graphics::quit(); });

  // Initialize graphics (creates window, renderer)
  graphics::initialize();

  // Start graphics loop (this blocks until quit and handles event processing)
  graphics::loop();

  std::cout << "Graphics Test Completed." << std::endl;
  return 0;
}

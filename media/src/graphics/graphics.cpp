#include "graphics/graphics.hpp"

#include "events/event_queue.hpp"
#include "events/events.hpp"

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

namespace graphics {
// Global graphics functions
std::unique_ptr<GraphicsContext> context_;
std::unique_ptr<Renderer> renderer_;
std::atomic<bool> quit_ = false;

void initialize() {
  context_ = std::make_unique<GraphicsContext>(LED_WIDTH, LED_HEIGHT);
  renderer_ = create_present_renderer();
  renderer_->init();
}

void loop() {
  std::cout << "Starting main graphics loop..." << std::endl;

  using clock = std::chrono::high_resolution_clock;
  auto last_frame_time = clock::now();

  while (!quit_) {
    // Calculate delta time
    auto current_time = clock::now();
    auto frame_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        current_time - last_frame_time);
    long long delta_time = frame_duration.count();
    last_frame_time = current_time;

    // Clear the context
    context_->clear();

    // Dispatch draw event
    // The ProcessManager and processes will listen to this and draw to the
    // context
    events::queue.enqueue(std::make_shared<DrawEvent>(delta_time));

    // Allow event queue to process events
    events::queue.process();

    // Render the context to the screen/matrix
    renderer_->draw(*context_);

    // Check for close/quit events
    if (renderer_->should_close()) {
      events::queue.enqueue(std::make_shared<StopEvent>());
    }

    // Limit frame rate (approx 30 FPS)
    std::this_thread::sleep_for(std::chrono::milliseconds(33));
  }

  std::cout << "Graphics loop completed." << std::endl;
}

void quit() {
  std::cout << "Quitting graphics system..." << std::endl;
  quit_ = true;
  if (renderer_) {
    renderer_->cleanup();
  }
  context_.reset();
  renderer_.reset();
}

void display_message(const std::string &message) {
  if (context_) {
    context_->clear();
    context_->draw_text(message, 10, 10, Pixel(255, 255, 255));
  }
}

void display_loading() {
  if (context_) {
    context_->clear();
    context_->draw_text("Loading...", 10, 10, Pixel(0, 255, 0));
  }
}

void display_timer(int minutes, int seconds) {
  if (!context_)
    return;

  context_->clear();

  std::string time_str = std::to_string(minutes) + ":" +
                         (seconds < 10 ? "0" : "") + std::to_string(seconds);
  context_->draw_text(time_str, 15, 30, Pixel(0, 255, 0)); // Green
}
} // namespace graphics
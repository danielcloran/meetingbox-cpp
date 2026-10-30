#include "graphics/graphics.hpp"
#include "events/event_queue.hpp"
#include "events/events.hpp"
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>

#ifdef RASPBERRY_PI
#include <ws2811.h>
#endif

namespace graphics {
// GraphicsContext implementation
GraphicsContext::GraphicsContext(int width, int height)
    : width_(width), height_(height), pixels_(width * height) {
  clear();
}

void GraphicsContext::clear(const Pixel &color) {
  for (auto &pixel : pixels_) {
    pixel = color;
  }
}

void GraphicsContext::set_pixel(int x, int y, const Pixel &color) {
  if (is_valid_position(x, y)) {
    pixels_[y * width_ + x] = color;
  }
}

void GraphicsContext::draw_rectangle(int x, int y, int width, int height,
                                     const Pixel &color) {
  for (int dy = 0; dy < height; ++dy) {
    for (int dx = 0; dx < width; ++dx) {
      set_pixel(x + dx, y + dy, color);
    }
  }
}

void GraphicsContext::draw_circle(int x, int y, int radius,
                                  const Pixel &color) {
  for (int dy = -radius; dy <= radius; ++dy) {
    for (int dx = -radius; dx <= radius; ++dx) {
      if (dx * dx + dy * dy <= radius * radius) {
        set_pixel(x + dx, y + dy, color);
      }
    }
  }
}

void GraphicsContext::draw_text(const std::string &text, int x, int y,
                                const Pixel &color) {
  // Simple text drawing - just draw rectangles for now
  for (size_t i = 0; i < text.length() && x + i * 6 < width_; ++i) {
    // Draw a simple representation of each character
    draw_rectangle(x + i * 6, y, 4, 7, color);
  }
}

void GraphicsContext::save_to_ppm(const std::string &filename) const {
  std::ofstream file(filename, std::ios::binary);
  if (!file)
    return;

  file << "P6\n" << width_ << " " << height_ << "\n255\n";
  for (const auto &pixel : pixels_) {
    file.write(reinterpret_cast<const char *>(&pixel.r), 1);
    file.write(reinterpret_cast<const char *>(&pixel.g), 1);
    file.write(reinterpret_cast<const char *>(&pixel.b), 1);
  }
}

bool GraphicsContext::is_valid_position(int x, int y) const {
  return x >= 0 && x < width_ && y >= 0 && y < height_;
}

// SDL2Renderer implementation
SDL2Renderer::SDL2Renderer()
    : window_(nullptr), renderer_(nullptr), texture_(nullptr), frame_count_(0),
      quit_(false) {}

SDL2Renderer::~SDL2Renderer() { cleanup(); }

void SDL2Renderer::init() {
  std::cout << "Initializing SDL2 renderer..." << std::endl;

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError()
              << std::endl;
    return;
  }

  // Create a window for the Box Layout (Cross shape)
  // Grid: 3x3 of 64x64 panels
  //   . T .
  //   L F R
  //   . B .
  // Width: 3 * 64
  // Height: 3 * 64
  int scale = 3; // Scale factor for visibility
  int window_w = 3 * 64 * scale;
  int window_h = 3 * 64 * scale;

  window_ = SDL_CreateWindow("MeetingBox LED Matrix (Dev)",
                             SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                             window_w, window_h, SDL_WINDOW_SHOWN);
  if (!window_) {
    std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError()
              << std::endl;
    SDL_Quit();
    return;
  }

  renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer_) {
    std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError()
              << std::endl;
    SDL_DestroyWindow(window_);
    SDL_Quit();
    return;
  }

  // Set logical size to match the 3x3 grid (192x192)
  SDL_RenderSetLogicalSize(renderer_, 192, 192);
  SDL_SetRenderDrawColor(renderer_, 0x10, 0x10, 0x10,
                         0xFF); // Dark gray background

  std::cout << "SDL2 renderer initialized successfully." << std::endl;
}

void SDL2Renderer::draw(const GraphicsContext &context) {
  if (!renderer_)
    return;

  SDL_SetRenderDrawColor(renderer_, 0x10, 0x10, 0x10, 0xFF);
  SDL_RenderClear(renderer_);

  // We need to map the linear 320x64 context to the 5 separate panels in the
  // cross layout Context Layout: [Top][Left][Front][Right][Back] (Assuming 64
  // pixels per panel) HOWEVER, the legacy mapping seemed to imply: Panel 0
  // (0-63): Left Panel 1 (64-127): Front Panel 2 (128-191): Right Panel 3
  // (192-255): Back Panel 4 (256-319): Top Let's stick to this deduction for
  // now.

  const auto &pixels = context.get_pixels();
  int panel_width = 64;
  int panel_height = 64;

  auto draw_panel = [&](int panel_idx, int dest_x, int dest_y) {
    int start_x = panel_idx * panel_width;

    for (int y = 0; y < panel_height; ++y) {
      for (int x = 0; x < panel_width; ++x) {
        int src_idx = y * context.get_width() + (start_x + x);
        if (src_idx < pixels.size()) {
          const auto &p = pixels[src_idx];
          SDL_SetRenderDrawColor(renderer_, p.r, p.g, p.b, 0xFF);
          SDL_RenderDrawPoint(renderer_, dest_x + x, dest_y + y);
        }
      }
    }
    // Draw border
    SDL_SetRenderDrawColor(renderer_, 0x40, 0x40, 0x40, 0xFF);
    SDL_Rect border = {dest_x, dest_y, panel_width, panel_height};
    SDL_RenderDrawRect(renderer_, &border);
  };

  // Draw panels in cross layout
  // Row 0: Top (Panel 4) at (64, 0)
  draw_panel(4, 64, 0);

  // Row 1: Left (Panel 0) at (0, 64)
  draw_panel(0, 0, 64);

  // Row 1: Front (Panel 1) at (64, 64)
  draw_panel(1, 64, 64);

  // Row 1: Right (Panel 2) at (128, 64)
  draw_panel(2, 128, 64);

  // Row 2: Back (Panel 3) at (64, 128)
  draw_panel(3, 64, 128);

  SDL_RenderPresent(renderer_);
  frame_count_++;
}

void SDL2Renderer::cleanup() {
  std::cout << "Cleaning up SDL2 renderer..." << std::endl;
  if (texture_) {
    SDL_DestroyTexture(texture_);
    texture_ = nullptr;
  }
  if (renderer_) {
    SDL_DestroyRenderer(renderer_);
    renderer_ = nullptr;
  }
  if (window_) {
    SDL_DestroyWindow(window_);
    window_ = nullptr;
  }
  SDL_Quit();
}

bool SDL2Renderer::should_close() {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT) {
      quit_ = true;
    }
  }
  return quit_;
}

void SDL2Renderer::draw_led_panels(const GraphicsContext &context) {}
void SDL2Renderer::draw_single_panel(const GraphicsContext &context,
                                     int panel_index, int x_offset) {}

// RpiRenderer implementation
RpiRenderer::RpiRenderer() : matrix_(nullptr) {}

RpiRenderer::~RpiRenderer() { cleanup(); }

#ifdef RASPBERRY_PI
// Global ws2811_t structure for RPi
ws2811_t output;
#endif

void RpiRenderer::init() {
  std::cout << "Initializing Raspberry Pi LED matrix renderer (WS2811)..."
            << std::endl;
#ifdef RASPBERRY_PI
  // Initialize ws2811 structure
  output.freq = WS2811_TARGET_FREQ;
  output.dmanum = 10;

  // Setup Channel 0
  output.channel[0].gpionum = 18; // Default to GPIO 18 (PWM0)
  output.channel[0].count =
      LED_WIDTH * LED_HEIGHT; // Total LEDs? Or per string?
  // Warning: 320*64 = 20480 LEDs is HUGE for a single channel.
  // Assuming the user's HAT handles parallel mapping or they have a specific
  // config. For now, mapping all onto channel 0 as a single stream.
  output.channel[0].invert = 0;
  output.channel[0].brightness = 255;
  output.channel[0].strip_type = WS2811_STRIP_GRB; // Typical for WS2812B

  // Disable Channel 1
  output.channel[1].gpionum = 0;
  output.channel[1].count = 0;
  output.channel[1].invert = 0;
  output.channel[1].brightness = 0;

  ws2811_return_t ret = ws2811_init(&output);
  if (ret != WS2811_SUCCESS) {
    std::cerr << "ws2811_init failed: " << ws2811_get_return_t_str(ret)
              << std::endl;
    return;
  }
  std::cout << "ws2811 initialized successfully." << std::endl;
#else
  std::cout << "Not running on RPi, skipping ws2811 init." << std::endl;
#endif
}

void RpiRenderer::draw(const GraphicsContext &context) {
#ifdef RASPBERRY_PI
  // Map context pixels to ws2811 channel 0
  // This is a direct mapping. If a custom mapper is needed (like the legacy
  // code had), it would go here.

  const auto &pixels = context.get_pixels();
  int count = std::min((int)pixels.size(), output.channel[0].count);

  for (int i = 0; i < count; ++i) {
    const auto &p = pixels[i];
    // WS2811 uses 32-bit int: 0xWWRRGGBB
    // We pack it according to strip type (library handles strip type mapping
    // usually, but input to led_string is 0x00RRGGBB)
    output.channel[0].leds[i] = (p.r << 16) | (p.g << 8) | p.b;
  }

  ws2811_return_t ret = ws2811_render(&output);
  if (ret != WS2811_SUCCESS) {
    std::cerr << "ws2811_render failed: " << ws2811_get_return_t_str(ret)
              << std::endl;
  }
#endif
}

void RpiRenderer::cleanup() {
  std::cout << "Cleaning up Raspberry Pi LED matrix renderer..." << std::endl;
#ifdef RASPBERRY_PI
  ws2811_fini(&output);
#endif
}

// Global graphics functions
std::unique_ptr<GraphicsContext> context_;
std::unique_ptr<Renderer> renderer_;
std::atomic<bool> quit_ = false;

void initialize() {
  context_ = std::make_unique<GraphicsContext>(LED_WIDTH, LED_HEIGHT);

#ifdef RASPBERRY_PI
  renderer_ = std::make_unique<RpiRenderer>();
#else
  renderer_ = std::make_unique<SDL2Renderer>();
#endif
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
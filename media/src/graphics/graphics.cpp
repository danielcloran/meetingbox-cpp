#include "graphics/graphics.hpp"
#include "events/event_queue.hpp"
#include "events/events.hpp"
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <thread>

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

  // Create a window to display 5 panels (64x64 each) side-by-side
  // Total width: 5 * 64 = 320 pixels, scale up for visibility
  int scale = 4; // Scale factor for visibility
  window_ = SDL_CreateWindow("MeetingBox LED Matrix", SDL_WINDOWPOS_UNDEFINED,
                             SDL_WINDOWPOS_UNDEFINED, LED_WIDTH * scale,
                             LED_HEIGHT * scale, SDL_WINDOW_SHOWN);
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

  // Set up scaling
  SDL_RenderSetLogicalSize(renderer_, LED_WIDTH, LED_HEIGHT);
  SDL_SetRenderDrawColor(renderer_, 0x00, 0x00, 0x00, 0xFF); // Black background

  std::cout << "SDL2 renderer initialized successfully." << std::endl;
}

void SDL2Renderer::draw(const GraphicsContext &context) {
  if (!renderer_)
    return;

  SDL_RenderClear(renderer_);

  // Create texture from pixel data
  if (!texture_) {
    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGB24,
                                 SDL_TEXTUREACCESS_STREAMING,
                                 context.get_width(), context.get_height());
  }

  if (texture_) {
    // Update texture with pixel data
    SDL_UpdateTexture(texture_, NULL, context.get_pixels().data(),
                      context.get_width() * sizeof(Pixel));

    // Render the texture
    SDL_RenderCopy(renderer_, texture_, NULL, NULL);
  }

  // Draw panel separators
  SDL_SetRenderDrawColor(renderer_, 0x40, 0x40, 0x40, 0xFF); // Gray separators
  for (int i = 1; i < LED_PANELS; ++i) {
    int x = i * 64;
    SDL_RenderDrawLine(renderer_, x, 0, x, LED_HEIGHT);
  }

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

void SDL2Renderer::draw_led_panels(const GraphicsContext &context) {
  // This is handled in the main draw() method
}

void SDL2Renderer::draw_single_panel(const GraphicsContext &context,
                                     int panel_index, int x_offset) {
  // This is handled in the main draw() method
}

// RpiRenderer implementation
RpiRenderer::RpiRenderer() : matrix_(nullptr) {}

RpiRenderer::~RpiRenderer() { cleanup(); }

void RpiRenderer::init() {
  std::cout << "Initializing Raspberry Pi LED matrix renderer..." << std::endl;
  // TODO: Initialize rpi-rgb-led-matrix here
  // RGBMatrix::Options options;
  // options.rows = 64;
  // options.cols = 64;
  // matrix_ = rgb_matrix::CreateMatrixFromOptions(options);
}

void RpiRenderer::draw(const GraphicsContext &context) {
  // TODO: Render context pixels to LED matrix
  // if (matrix_) {
  //     FrameCanvas* canvas = matrix_->CreateFrameCanvas();
  //     for (int y = 0; y < context.get_height(); ++y) {
  //         for (int x = 0; x < context.get_width(); ++x) {
  //             const Pixel& p = context.get_pixels()[y * context.get_width() +
  //             x]; canvas->SetPixel(x, y, p.r, p.g, p.b);
  //         }
  //     }
  //     matrix_->SwapOnVSync(canvas);
  // }
  std::cout << "RPi rendering frame..." << std::endl;
}

void RpiRenderer::cleanup() {
  std::cout << "Quitting Raspberry Pi LED matrix renderer." << std::endl;
  // TODO: Clean up rpi-rgb-led-matrix
  // if (matrix_) {
  //     matrix_->Stop();
  //     delete matrix_;
  //     matrix_ = nullptr;
  // }
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
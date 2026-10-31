#include "graphics/rpi_renderer.hpp"

#include "graphics/rpi/pixel_mapper.hpp"

#include "led-matrix.h"

#include <algorithm>
#include <iostream>
#include <vector>

namespace graphics {
namespace {

// Same chain the orphaned HUB75 driver used. show_refresh_rate stays off:
// the library prints that counter on stdout, and the MCP server owns stdout.
rgb_matrix::RGBMatrix::Options matrix_options() {
  rgb_matrix::RGBMatrix::Options options;
  options.hardware_mapping = "regular";
  options.panel_type = "FM6126A";
  options.rows = 64;
  options.cols = 64;
  options.chain_length = 2;
  options.parallel = 3;
  options.show_refresh_rate = false;
  options.pwm_lsb_nanoseconds = 50;
  options.led_rgb_sequence = "BGR";
  return options;
}

rgb_matrix::RuntimeOptions runtime_options() {
  rgb_matrix::RuntimeOptions options;
  options.gpio_slowdown = 4;
  return options;
}

}  // namespace

struct RpiRenderer::Hub75 {
  rgb_matrix::RGBMatrix *canvas = nullptr;
  std::vector<Pixel> previous;
};

RpiRenderer::RpiRenderer() = default;

RpiRenderer::~RpiRenderer() { cleanup(); }

void RpiRenderer::init() {
  ready_ = false;
  cleanup();
  hub_ = new Hub75();

  const rgb_matrix::RGBMatrix::Options options = matrix_options();
  hub_->canvas =
      rgb_matrix::CreateMatrixFromOptions(options, runtime_options());
  if (hub_->canvas == nullptr) {
    std::cerr << "Failed to create RGBMatrix canvas\n";
    cleanup();
    return;
  }

  // Copies the mapping. The mapper does not need to outlive this call.
  CustomMapper mapper;
  if (!hub_->canvas->ApplyPixelMapper(&mapper)) {
    std::cerr << "HUB75 pixel mapper rejected the 320x64 strip\n";
    cleanup();
    return;
  }

  std::cerr << "HUB75 present " << hub_->canvas->width() << "x"
            << hub_->canvas->height()
            << " FM6126A regular chain 2 parallel 3\n";
  ready_ = true;
}

bool RpiRenderer::init_present() {
  init();
  return ready_;
}

void RpiRenderer::draw(const GraphicsContext &context) {
  if (!ready_ || hub_ == nullptr || hub_->canvas == nullptr)
    return;

  // Host frame, not a window surface: 320x64 RGB888.
  // Left | Front | Top | Right | Back. Lid is panel index 2.
  const auto &pixels = context.get_pixels();
  const int src_width = context.get_width();
  const int src_height = context.get_height();
  const int width = std::min(src_width, hub_->canvas->width());
  const int height = std::min(src_height, hub_->canvas->height());
  if (static_cast<int>(hub_->previous.size()) != src_width * src_height)
    hub_->previous.assign(static_cast<size_t>(src_width * src_height), Pixel());

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      const int index = y * src_width + x;
      const Pixel &pixel = pixels[static_cast<size_t>(index)];
      Pixel &prior = hub_->previous[static_cast<size_t>(index)];
      if (pixel == prior)
        continue;
      hub_->canvas->SetPixel(x, y, pixel.r, pixel.g, pixel.b);
      prior = pixel;
    }
  }
}

void RpiRenderer::cleanup() {
  if (hub_ == nullptr)
    return;
  delete hub_->canvas;
  delete hub_;
  hub_ = nullptr;
  ready_ = false;
}

}  // namespace graphics

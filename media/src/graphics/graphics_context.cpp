#include "graphics/graphics.hpp"

#include <fstream>
#include <string>

namespace graphics {
GraphicsContext::GraphicsContext(int width, int height)
    : pixels_(width * height), width_(width), height_(height) {
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

bool GraphicsContext::save_to_ppm(const std::string &filename) const {
  std::ofstream file(filename, std::ios::binary);
  if (!file)
    return false;

  file << "P6\n" << width_ << " " << height_ << "\n255\n";
  for (const auto &pixel : pixels_) {
    file.write(reinterpret_cast<const char *>(&pixel.r), 1);
    file.write(reinterpret_cast<const char *>(&pixel.g), 1);
    file.write(reinterpret_cast<const char *>(&pixel.b), 1);
  }
  return static_cast<bool>(file);
}

bool GraphicsContext::is_valid_position(int x, int y) const {
  return x >= 0 && x < width_ && y >= 0 && y < height_;
}
}  // namespace graphics

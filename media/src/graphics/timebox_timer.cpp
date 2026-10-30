#include "graphics/timebox_timer.hpp"

namespace graphics {

void paint_timebox_timer(GraphicsContext &canvas, int remaining_ms,
                         int total_ms) {
  float remaining = 0.f;
  if (total_ms > 0 && remaining_ms > 0) {
    remaining = static_cast<float>(remaining_ms) / static_cast<float>(total_ms);
  }
  const int height = canvas.get_height();
  const int width = canvas.get_width();
  for (int row = 0; row < height; ++row) {
    const Pixel color(0, 0, timebox_row_blue(row, height, remaining));
    for (int x = 0; x < width; ++x) {
      canvas.set_pixel(x, row, color);
    }
  }
}

}  // namespace graphics

#include "graphics/timebox_timer.hpp"

#include <array>

namespace graphics {
namespace {

constexpr int kSpiralCount = kPanelSize * kPanelSize;

const std::array<int, kSpiralCount> &spiral_ranks() {
  static const std::array<int, kSpiralCount> ranks = [] {
    std::array<int, kSpiralCount> out{};
    int rank = 0;
    int top = 0;
    int bottom = kPanelSize - 1;
    int left = 0;
    int right = kPanelSize - 1;
    while (top <= bottom && left <= right) {
      for (int x = left; x <= right; ++x)
        out[top * kPanelSize + x] = rank++;
      ++top;
      for (int y = top; y <= bottom; ++y)
        out[y * kPanelSize + right] = rank++;
      --right;
      if (top <= bottom) {
        for (int x = right; x >= left; --x)
          out[bottom * kPanelSize + x] = rank++;
        --bottom;
      }
      if (left <= right) {
        for (int y = bottom; y >= top; --y)
          out[y * kPanelSize + left] = rank++;
        ++left;
      }
    }
    return out;
  }();
  return ranks;
}

void paint_side_fade(GraphicsContext &canvas, float remaining) {
  const int height = canvas.get_height();
  const int width = canvas.get_width();
  for (int row = 0; row < height; ++row) {
    const Pixel color(0, 0, timebox_row_blue(row, height, remaining));
    for (int x = 0; x < width; ++x)
      canvas.set_pixel(x, row, color);
  }
}

void paint_lid_spiral(GraphicsContext &canvas, int origin_x, float remaining) {
  const int dark = remaining <= 0.f
                       ? kSpiralCount
                       : remaining >= 1.f
                             ? 0
                             : static_cast<int>((1.f - remaining) * kSpiralCount);
  const int span = kSpiralCount - dark;
  const auto &ranks = spiral_ranks();
  for (int y = 0; y < kPanelSize; ++y) {
    for (int x = 0; x < kPanelSize; ++x) {
      const int rank = ranks[y * kPanelSize + x];
      uint8_t blue = 0;
      if (rank >= dark && span > 0) {
        const int along = rank - dark;
        const int denom = span > 1 ? span - 1 : 1;
        blue = static_cast<uint8_t>(40 + (215 * along) / denom);
      }
      canvas.set_pixel(origin_x + x, y, Pixel(0, 0, blue));
    }
  }
}

}  // namespace

int timebox_spiral_rank(int local_x, int local_y) {
  if (local_x < 0 || local_y < 0 || local_x >= kPanelSize || local_y >= kPanelSize)
    return 0;
  return spiral_ranks()[local_y * kPanelSize + local_x];
}

uint8_t timebox_lid_blue(int local_x, int local_y, float remaining_fraction) {
  if (remaining_fraction < 0.f)
    remaining_fraction = 0.f;
  if (remaining_fraction > 1.f)
    remaining_fraction = 1.f;
  const int rank = timebox_spiral_rank(local_x, local_y);
  const int dark = remaining_fraction <= 0.f
                       ? kSpiralCount
                       : remaining_fraction >= 1.f
                             ? 0
                             : static_cast<int>((1.f - remaining_fraction) * kSpiralCount);
  if (rank < dark)
    return 0;
  const int span = kSpiralCount - dark;
  if (span <= 0)
    return 0;
  const int along = rank - dark;
  const int denom = span > 1 ? span - 1 : 1;
  return static_cast<uint8_t>(40 + (215 * along) / denom);
}

void paint_timebox_timer(GraphicsContext &canvas, int remaining_ms, int total_ms) {
  const float remaining = timebox_remaining_fraction(remaining_ms, total_ms);
  paint_side_fade(canvas, remaining);
  const bool has_lid = canvas.get_width() >= kPanelSize * (kTimeboxTopPanel + 1) &&
                       canvas.get_height() == kPanelSize;
  if (has_lid)
    paint_lid_spiral(canvas, kTimeboxTopPanel * kPanelSize, remaining);
}

}  // namespace graphics

#ifndef TIMEBOX_TIMER_HPP
#define TIMEBOX_TIMER_HPP

#include "graphics/graphics.hpp"

namespace graphics {

// Blue channel for one row of the original TimeBox side-face countdown.
//
//   row_percentage = (height - row) / height     // row 0 is the top of a wall
//   brightness = 1 - (1 - remaining) / (1 - row_percentage)
inline uint8_t timebox_row_blue(int row, int height, float remaining_fraction) {
  if (height <= 0 || row < 0 || row >= height)
    return 0;
  if (remaining_fraction < 0.f)
    remaining_fraction = 0.f;
  if (remaining_fraction > 1.f)
    remaining_fraction = 1.f;

  const float row_percentage =
      static_cast<float>(height - row) / static_cast<float>(height);
  float brightness;
  if (row_percentage >= 1.f) {
    brightness = remaining_fraction >= 1.f ? 1.f : 0.f;
  } else {
    brightness = 1.f - (1.f - remaining_fraction) / (1.f - row_percentage);
  }
  if (brightness < 0.f)
    brightness = 0.f;
  if (brightness > 1.f)
    brightness = 1.f;
  return static_cast<uint8_t>(255.f * brightness);
}

// Middle panel of the 5-wide strip, and the center of the unfolded cross.
constexpr int kTimeboxTopPanel = 2;

inline float timebox_remaining_fraction(int remaining_ms, int total_ms) {
  if (total_ms <= 0 || remaining_ms <= 0)
    return 0.f;
  float remaining = static_cast<float>(remaining_ms) / static_cast<float>(total_ms);
  if (remaining > 1.f)
    remaining = 1.f;
  return remaining;
}

// Clockwise spiral rank on one 64x64 face. 0 is the outer start at (0, 0);
// the last rank is the center. The countdown drains along this order.
int timebox_spiral_rank(int local_x, int local_y);

// Lid pixel. Outer ranks go dark first; the center stays lit longest.
// 0 when the meeting is over.
uint8_t timebox_lid_blue(int local_x, int local_y, float remaining_fraction);

// Walls: original blue row-fade. Lid (strip panel 2): spiral into the center.
void paint_timebox_timer(GraphicsContext &canvas, int remaining_ms, int total_ms);

}  // namespace graphics

#endif  // TIMEBOX_TIMER_HPP

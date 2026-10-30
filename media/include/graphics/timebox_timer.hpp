#ifndef TIMEBOX_TIMER_HPP
#define TIMEBOX_TIMER_HPP

#include "graphics/graphics.hpp"

namespace graphics {

// Blue channel for one row of the original TimeBox countdown.
//
// This is the row-fade from TimerProcess::draw after "timer correct direction"
// (the interrupt timer paints a 64px-tall field, not a clock face):
//
//   row_percentage = (height - row) / height     // row 0 is the top
//   brightness = 1 - (1 - remaining) / (1 - row_percentage)
//
// remaining_fraction is time remaining / duration, clamped to [0, 1].
// The top row (row_percentage == 1) is full blue only while the timer is full.
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
  // SDL_SetRenderDrawColor truncates the float channel the same way.
  return static_cast<uint8_t>(255.f * brightness);
}

// Paint the countdown across the LED strip. Each row is one blue level, so
// every 64x64 face shows the same draining field the original timer drew.
void paint_timebox_timer(GraphicsContext &canvas, int remaining_ms, int total_ms);

}  // namespace graphics

#endif  // TIMEBOX_TIMER_HPP

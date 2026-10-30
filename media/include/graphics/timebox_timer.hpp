#ifndef TIMEBOX_TIMER_HPP
#define TIMEBOX_TIMER_HPP

#include "graphics/graphics.hpp"

namespace graphics {

// Blue channel for one row of the original TimeBox side-face countdown.
//
// Row-fade from TimerProcess::draw after "timer correct direction". The four
// wall faces use this. The lid does not: nothing in the TimeBox history drew
// a different top, so the lid is a status meter instead of a fifth copy.
//
//   row_percentage = (height - row) / height     // row 0 is the top of a side
//   brightness = 1 - (1 - remaining) / (1 - row_percentage)
//
// remaining_fraction is time remaining / duration, clamped to [0, 1].
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

// Strip panel 4 is the lid (box top). Looking down at a box on a table,
// that face is a meeting-status lamp plus a remaining-time bar.
constexpr int kTimeboxTopPanel = 4;
constexpr int kTimeboxLidBarInset = 6;
constexpr int kTimeboxLidBarY = 40;
constexpr int kTimeboxLidBarH = 10;
constexpr int kTimeboxLidLamp = 14;
constexpr int kTimeboxLidLampY = 8;
constexpr float kTimeboxLidEndingFraction = 0.25f;

constexpr Pixel kTimeboxLidBackground{12, 20, 32};
constexpr Pixel kTimeboxLidTrack{42, 54, 70};
constexpr Pixel kTimeboxLidFill{255, 196, 72};
constexpr Pixel kTimeboxLidInMeeting{48, 210, 116};
constexpr Pixel kTimeboxLidEnding{255, 150, 40};
constexpr Pixel kTimeboxLidOver{220, 52, 58};

inline float timebox_remaining_fraction(int remaining_ms, int total_ms) {
  if (total_ms <= 0 || remaining_ms <= 0)
    return 0.f;
  float remaining = static_cast<float>(remaining_ms) / static_cast<float>(total_ms);
  if (remaining > 1.f)
    remaining = 1.f;
  return remaining;
}

inline int timebox_lid_bar_width() {
  return kPanelSize - 2 * kTimeboxLidBarInset;
}

// Lit pixels of the lid bar, from the left. 0 at the end, full width at the start.
inline int timebox_lid_lit_width(float remaining_fraction) {
  if (remaining_fraction <= 0.f)
    return 0;
  if (remaining_fraction > 1.f)
    remaining_fraction = 1.f;
  return static_cast<int>(timebox_lid_bar_width() * remaining_fraction + 0.5f);
}

inline Pixel timebox_lid_phase(float remaining_fraction) {
  if (remaining_fraction <= 0.f)
    return kTimeboxLidOver;
  if (remaining_fraction <= kTimeboxLidEndingFraction)
    return kTimeboxLidEnding;
  return kTimeboxLidInMeeting;
}

// Sides: original blue row-fade. Lid (panel 4 of a 320x64 strip): status lamp
// and a horizontal remaining bar. A shorter canvas is sides only.
void paint_timebox_timer(GraphicsContext &canvas, int remaining_ms, int total_ms);

}  // namespace graphics

#endif  // TIMEBOX_TIMER_HPP

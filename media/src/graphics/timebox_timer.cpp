#include "graphics/timebox_timer.hpp"

namespace graphics {
namespace {

void paint_side_fade(GraphicsContext &canvas, int width, float remaining) {
  const int height = canvas.get_height();
  for (int row = 0; row < height; ++row) {
    const Pixel color(0, 0, timebox_row_blue(row, height, remaining));
    for (int x = 0; x < width; ++x)
      canvas.set_pixel(x, row, color);
  }
}

void paint_lid(GraphicsContext &canvas, int origin_x, float remaining) {
  canvas.draw_rectangle(origin_x, 0, kPanelSize, kPanelSize, kTimeboxLidBackground);

  const int lamp_x = origin_x + (kPanelSize - kTimeboxLidLamp) / 2;
  canvas.draw_rectangle(lamp_x, kTimeboxLidLampY, kTimeboxLidLamp, kTimeboxLidLamp,
                        timebox_lid_phase(remaining));

  const int bar_w = timebox_lid_bar_width();
  const int bar_x = origin_x + kTimeboxLidBarInset;
  canvas.draw_rectangle(bar_x, kTimeboxLidBarY, bar_w, kTimeboxLidBarH, kTimeboxLidTrack);
  const int lit = timebox_lid_lit_width(remaining);
  if (lit > 0) {
    canvas.draw_rectangle(bar_x, kTimeboxLidBarY, lit, kTimeboxLidBarH, kTimeboxLidFill);
  }
}

}  // namespace

void paint_timebox_timer(GraphicsContext &canvas, int remaining_ms, int total_ms) {
  const float remaining = timebox_remaining_fraction(remaining_ms, total_ms);
  const bool has_lid = canvas.get_width() >= kPanelSize * (kTimeboxTopPanel + 1) &&
                       canvas.get_height() == kPanelSize;
  const int side_width = has_lid ? kPanelSize * kTimeboxTopPanel : canvas.get_width();
  paint_side_fade(canvas, side_width, remaining);
  if (has_lid)
    paint_lid(canvas, kTimeboxTopPanel * kPanelSize, remaining);
}

}  // namespace graphics

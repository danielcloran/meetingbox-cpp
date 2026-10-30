#include "graphics/graphics.hpp"

namespace graphics {
namespace {

// Strip order is the graphics-test order: left, front, right, back, top.
// Destinations are the unfolded cross, not the Raspberry Pi chain mapper.
constexpr BoxPanelPlacement kPlacements[kBoxPanelCount] = {
    {4, 64, 0},    // top
    {0, 0, 64},    // left
    {1, 64, 64},   // front
    {2, 128, 64},  // right
    {3, 64, 128},  // back
};

static_assert(LED_PANELS == kBoxPanelCount, "one placement per face");
static_assert(LED_WIDTH == kPanelSize * LED_PANELS, "strip is five panels wide");
static_assert(LED_HEIGHT == kPanelSize, "strip is one panel tall");

}  // namespace

const BoxPanelPlacement *box_panel_placements() { return kPlacements; }

void paint_box_layout(const GraphicsContext &strip, GraphicsContext &box) {
  box.clear(kBoxBackground);
  const std::vector<Pixel> &pixels = strip.get_pixels();
  const int strip_width = strip.get_width();
  const int strip_height = strip.get_height();

  for (int i = 0; i < kBoxPanelCount; ++i) {
    const BoxPanelPlacement &panel = kPlacements[i];
    for (int y = 0; y < kPanelSize; ++y) {
      for (int x = 0; x < kPanelSize; ++x) {
        const int src_x = panel.strip_index * kPanelSize + x;
        const int src_y = y;
        if (src_x < 0 || src_y < 0 || src_x >= strip_width ||
            src_y >= strip_height) {
          continue;
        }
        box.set_pixel(panel.dest_x + x, panel.dest_y + y,
                      pixels[src_y * strip_width + src_x]);
      }
    }
  }
}

void paint_graphics_test_pattern(GraphicsContext &strip) {
  strip.clear(Pixel(0, 0, 0));

  struct Panel {
    int index;
    Pixel color;
    Pixel text;
    const char *label;
  };
  const Panel panels[] = {
      {0, Pixel(255, 0, 0), Pixel(255, 255, 255), "LEFT"},
      {1, Pixel(0, 255, 0), Pixel(0, 0, 0), "FRONT"},
      {2, Pixel(0, 0, 255), Pixel(255, 255, 255), "RIGHT"},
      {3, Pixel(255, 255, 0), Pixel(0, 0, 0), "BACK"},
      {4, Pixel(255, 0, 255), Pixel(255, 255, 255), "TOP"},
  };
  for (const Panel &panel : panels) {
    strip.draw_rectangle(panel.index * kPanelSize, 0, kPanelSize, kPanelSize,
                         panel.color);
    strip.draw_text(panel.label, panel.index * kPanelSize + 10, 20, panel.text);
  }
}

}  // namespace graphics

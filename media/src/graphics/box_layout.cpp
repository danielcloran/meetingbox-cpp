#include "graphics/graphics.hpp"

namespace graphics {
namespace {

// Physical lid is the middle panel of the 320-wide strip (index 2), and the
// center of this cross. The Pi driver (chain 2 x parallel 3) only numbers
// panels 1..5 onto a 2-column grid; it does not name cube faces. A 2025
// graphics split guessed index 4 was the top. That cell is the back wall.
//
// Strip: [Left][Front][Top][Right][Back]
// Cross, lid in the middle (the face you look down on):
//        Back
//   Left Top  Right
//        Front
constexpr BoxPanelPlacement kPlacements[kBoxPanelCount] = {
    {2, 64, 64},   // top / lid
    {0, 0, 64},    // left
    {3, 128, 64},  // right
    {1, 64, 128},  // front
    {4, 64, 0},    // back
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
      {2, Pixel(0, 0, 255), Pixel(255, 255, 255), "TOP"},
      {3, Pixel(255, 255, 0), Pixel(0, 0, 0), "RIGHT"},
      {4, Pixel(255, 0, 255), Pixel(255, 255, 255), "BACK"},
  };
  for (const Panel &panel : panels) {
    strip.draw_rectangle(panel.index * kPanelSize, 0, kPanelSize, kPanelSize,
                         panel.color);
    strip.draw_text(panel.label, panel.index * kPanelSize + 10, 20, panel.text);
  }
}

}  // namespace graphics

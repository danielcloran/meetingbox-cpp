#ifndef BOX_LAYOUT_SPEC_HPP
#define BOX_LAYOUT_SPEC_HPP

// Expected unfolded-box colours for the graphics test. These coordinates are
// the contract; they are not read from box_panel_placements().
//
//   . T .
//   L F R
//   . B .

#include "graphics/graphics.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

namespace graphics {
namespace testspec {

inline std::string pixel_desc(const Pixel &pixel) {
  return "(" + std::to_string(pixel.r) + "," + std::to_string(pixel.g) + "," +
         std::to_string(pixel.b) + ")";
}

inline constexpr Pixel kSpecBackground{0x10, 0x10, 0x10};

struct Region {
  int x;
  int y;
  int w;
  int h;
  Pixel color;
};

// Cross positions. Colours match the graphics test panels.
inline constexpr Region kCrossPanels[] = {
    {64, 0, 64, 64, Pixel{255, 0, 255}},    // top
    {0, 64, 64, 64, Pixel{255, 0, 0}},      // left
    {64, 64, 64, 64, Pixel{0, 255, 0}},     // front
    {128, 64, 64, 64, Pixel{0, 0, 255}},    // right
    {64, 128, 64, 64, Pixel{255, 255, 0}},  // back
};

inline Pixel expected_solid_pixel(int x, int y) {
  for (const Region &region : kCrossPanels) {
    if (x >= region.x && y >= region.y && x < region.x + region.w &&
        y < region.y + region.h) {
      return region.color;
    }
  }
  return kSpecBackground;
}

inline bool check_dimensions(std::string &error) {
  if (LED_WIDTH != 320 || LED_HEIGHT != 64 || LED_PANELS != 5 ||
      kPanelSize != 64 || kBoxWidth != 192 || kBoxHeight != 192) {
    error = "LED strip or box dimensions drifted from 5x64 and 192x192\n";
    return false;
  }
  if (kBoxBackground != kSpecBackground) {
    error = "box background drifted from rgb(16,16,16)\n";
    return false;
  }
  return true;
}

inline bool check_solid_cross(GraphicsContext &painted, std::string &error) {
  GraphicsContext strip(LED_WIDTH, LED_HEIGHT);
  strip.clear(Pixel(0, 0, 0));
  // Strip order is independent of the cross destinations above.
  strip.draw_rectangle(0, 0, 64, 64, Pixel(255, 0, 0));
  strip.draw_rectangle(64, 0, 64, 64, Pixel(0, 255, 0));
  strip.draw_rectangle(128, 0, 64, 64, Pixel(0, 0, 255));
  strip.draw_rectangle(192, 0, 64, 64, Pixel(255, 255, 0));
  strip.draw_rectangle(256, 0, 64, 64, Pixel(255, 0, 255));

  const Pixel strip_samples[][2] = {
      {strip.get_pixels()[0], Pixel(255, 0, 0)},
      {strip.get_pixels()[64], Pixel(0, 255, 0)},
      {strip.get_pixels()[128], Pixel(0, 0, 255)},
      {strip.get_pixels()[192], Pixel(255, 255, 0)},
      {strip.get_pixels()[256], Pixel(255, 0, 255)},
      {strip.get_pixels()[63 * LED_WIDTH + 319], Pixel(255, 0, 255)},
  };
  for (const auto &sample : strip_samples) {
    if (sample[0] != sample[1]) {
      error = "strip draw_rectangle did not fill a panel: got " +
              pixel_desc(sample[0]) + " expected " + pixel_desc(sample[1]) +
              "\n";
      return false;
    }
  }

  paint_box_layout(strip, painted);
  if (painted.get_width() != 192 || painted.get_height() != 192) {
    error = "painted box is not 192x192\n";
    return false;
  }

  const auto &pixels = painted.get_pixels();
  int mismatches = 0;
  for (int y = 0; y < 192; ++y) {
    for (int x = 0; x < 192; ++x) {
      const Pixel expected = expected_solid_pixel(x, y);
      const Pixel &got = pixels[y * 192 + x];
      if (got != expected) {
        if (mismatches < 8) {
          error += "solid (" + std::to_string(x) + "," + std::to_string(y) +
                   ") got " + pixel_desc(got) + " expected " +
                   pixel_desc(expected) + "\n";
        }
        ++mismatches;
      }
    }
  }
  if (mismatches != 0) {
    error += std::to_string(mismatches) + " solid-layout pixels mismatched\n";
    return false;
  }
  return true;
}

struct Anchor {
  int x;
  int y;
  Pixel color;
  const char *name;
};

inline bool check_anchors(const GraphicsContext &box, const Anchor *anchors,
                          int count, std::string &error) {
  if (box.get_width() != 192 || box.get_height() != 192) {
    error += "anchor buffer is not 192x192\n";
    return false;
  }
  const auto &pixels = box.get_pixels();
  bool ok = true;
  for (int i = 0; i < count; ++i) {
    const Anchor &anchor = anchors[i];
    const Pixel &got = pixels[anchor.y * 192 + anchor.x];
    if (got != anchor.color) {
      error += std::string(anchor.name) + " at (" + std::to_string(anchor.x) +
               "," + std::to_string(anchor.y) + ") got " + pixel_desc(got) +
               " expected " + pixel_desc(anchor.color) + "\n";
      ok = false;
    }
  }
  return ok;
}

// Samples sit clear of the 4x7 glyphs drawn at local (10, 20).
inline bool check_pattern_anchors(const GraphicsContext &box, std::string &error) {
  const Anchor anchors[] = {
      {48, 112, Pixel{255, 0, 0}, "left panel"},
      {112, 112, Pixel{0, 255, 0}, "front panel"},
      {176, 112, Pixel{0, 0, 255}, "right panel"},
      {112, 176, Pixel{255, 255, 0}, "back panel"},
      {112, 48, Pixel{255, 0, 255}, "top panel"},
      {10, 84, Pixel{255, 255, 255}, "LEFT text"},
      {74, 84, Pixel{0, 0, 0}, "FRONT text"},
      {138, 84, Pixel{255, 255, 255}, "RIGHT text"},
      {74, 148, Pixel{0, 0, 0}, "BACK text"},
      {74, 20, Pixel{255, 255, 255}, "TOP text"},
      {32, 32, kSpecBackground, "top-left empty"},
      {160, 32, kSpecBackground, "top-right empty"},
      {32, 160, kSpecBackground, "bottom-left empty"},
      {160, 160, kSpecBackground, "bottom-right empty"},
  };
  return check_anchors(box, anchors, sizeof(anchors) / sizeof(anchors[0]), error);
}

inline bool buffers_equal(const GraphicsContext &got, const GraphicsContext &expected,
                          std::string &error) {
  if (got.get_width() != expected.get_width() ||
      got.get_height() != expected.get_height()) {
    error += "SDL capture size does not match the layout buffer\n";
    return false;
  }
  const auto &got_pixels = got.get_pixels();
  const auto &expected_pixels = expected.get_pixels();
  const int width = got.get_width();
  const int height = got.get_height();
  int mismatches = 0;
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      const Pixel &actual = got_pixels[y * width + x];
      const Pixel &want = expected_pixels[y * width + x];
      if (actual != want) {
        if (mismatches < 8) {
          error += "SDL (" + std::to_string(x) + "," + std::to_string(y) +
                   ") got " + pixel_desc(actual) + " layout " + pixel_desc(want) +
                   "\n";
        }
        ++mismatches;
      }
    }
  }
  if (mismatches != 0) {
    error += std::to_string(mismatches) +
             " SDL pixels differ from paint_box_layout\n";
    return false;
  }
  return true;
}

inline std::string solid_ppm_path(const std::string &ppm_path) {
  const auto dot = ppm_path.rfind('.');
  if (dot == std::string::npos) {
    return ppm_path + "_solid";
  }
  return ppm_path.substr(0, dot) + "_solid" + ppm_path.substr(dot);
}

// Returns 0 when the strip maps to the cross and SDL readback matches it.
inline int run_box_paint_checks(const std::string &ppm_path) {
  setenv("SDL_VIDEODRIVER", "dummy", 1);

  std::string error;
  if (!check_dimensions(error)) {
    std::cerr << "FAIL dimensions: " << error;
    return 1;
  }

  GraphicsContext solid(kBoxWidth, kBoxHeight);
  if (!check_solid_cross(solid, error)) {
    std::cerr << "FAIL solid cross layout:\n" << error;
    return 1;
  }
  std::cout << "PASS solid cross layout (192x192, 5 panels + empty corners)\n";

  GraphicsContext strip(LED_WIDTH, LED_HEIGHT);
  paint_graphics_test_pattern(strip);
  GraphicsContext laid_out(kBoxWidth, kBoxHeight);
  paint_box_layout(strip, laid_out);
  if (!check_pattern_anchors(laid_out, error)) {
    std::cerr << "FAIL graphics-test pattern anchors:\n" << error;
    return 1;
  }
  std::cout << "PASS graphics-test pattern anchors\n";

  GraphicsContext captured(kBoxWidth, kBoxHeight);
  if (!capture_painted_box(strip, captured, error)) {
    std::cerr << "FAIL SDL headless capture: " << error << "\n";
    return 1;
  }
  if (!buffers_equal(captured, laid_out, error)) {
    std::cerr << "FAIL SDL framebuffer:\n" << error;
    return 1;
  }
  if (!check_pattern_anchors(captured, error)) {
    std::cerr << "FAIL SDL panel colours:\n" << error;
    return 1;
  }
  std::cout << "PASS SDL software renderer readback matches the box layout\n";

  std::string output = ppm_path;
  if (output.empty()) {
    if (const char *dir = std::getenv("TEST_UNDECLARED_OUTPUTS_DIR")) {
      output = std::string(dir) + "/box_layout.ppm";
    }
  }
  if (!output.empty()) {
    const std::string solid_path = solid_ppm_path(output);
    if (!captured.save_to_ppm(output)) {
      std::cerr << "FAIL could not write " << output << "\n";
      return 1;
    }
    if (!solid.save_to_ppm(solid_path)) {
      std::cerr << "FAIL could not write " << solid_path << "\n";
      return 1;
    }
    std::cout << "Wrote " << output << "\n";
    std::cout << "Wrote " << solid_path << "\n";
  }
  return 0;
}

}  // namespace testspec
}  // namespace graphics

#endif

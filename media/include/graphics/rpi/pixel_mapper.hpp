#ifndef PIXEL_MAPPER_H
#define PIXEL_MAPPER_H

#include <pixel-mapper.h>

// Visible canvas is the host strip: 320x64, five 64x64 panels
// Left | Front | Top | Right | Back. The lid is strip index 2.
// This mapper only places that strip onto the HUB75 chains
// (cols 64, chain 2, parallel 3 -> 128x192). It does not reorder panels.
class CustomMapper : public rgb_matrix::PixelMapper {
 public:
  const char *GetName() const override { return "BoxMapper"; }

  bool GetSizeMapping(int matrix_width, int matrix_height, int *visible_width,
                      int *visible_height) const override {
    (void)matrix_width;
    (void)matrix_height;
    *visible_height = 64;
    *visible_width = 320;
    return true;
  }

  //   1, 2
  //   3, 4
  //   -, 5
  // x,y
  // 0,0 -> 0,0        panel 1
  // 64, 0 -> 64, 0    panel 2
  // 128, 0 -> 0, 64   panel 3
  // 192, 0 -> 64, 64  panel 4
  // 256, 0 -> 128, 64 panel 5
  void MapVisibleToMatrix(int matrix_width, int matrix_height, int visible_x,
                          int visible_y, int *matrix_x,
                          int *matrix_y) const override {
    (void)matrix_width;
    (void)matrix_height;
    // Panel 1 and 2 are fine
    *matrix_y = visible_y;
    *matrix_x = visible_x;
    // Panel 3 and 4 are one row up
    if (visible_x >= 128 && visible_x <= 255) {
      *matrix_x = visible_x - 128;
      *matrix_y = visible_y + 64;
    }
    // Panel 5 is two rows up one block over
    else if (visible_x >= 256 && visible_x <= 319) {
      *matrix_x = visible_x - 192;
      *matrix_y = visible_y + 128;
    }
  }
};

#endif /* PIXEL_MAPPER_H */

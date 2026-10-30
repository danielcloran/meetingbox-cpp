#ifndef PIXEL_MAPPER_H
#define PIXEL_MAPPER_H

#include <pixel-mapper.h>

class CustomMapper : public rgb_matrix::PixelMapper
{
    virtual const char *GetName() const { return "BoxMapper"; }
    virtual bool GetSizeMapping(int matrix_width, int matrix_height,
                                int *visible_width, int *visible_height) const
    {
        *visible_height = 64;
        *visible_width = 320;
        return true;
    }

    // windowCanvas->DrawText(defaultFont, 10, 10, Color(120,10,10), NULL, "1");
    // windowCanvas->DrawText(defaultFont, 74, 10, Color(120,10,10), NULL, "2");
    // windowCanvas->DrawText(defaultFont, 10, 74, Color(120,10,10), NULL, "3");
    // windowCanvas->DrawText(defaultFont, 74, 74, Color(120,10,10), NULL, "4");
    // windowCanvas->DrawText(defaultFont, 74, 138, Color(120,10,10), NULL, "5");

    //   1, 2
    //   3, 4
    //   -, 5
    // x,y
    // 0,0 -> 0,0        panel 1
    // 64, 0 -> 64, 0    panel 2
    // 128, 0 -> 0, 64   panel 3
    // 192, 0 -> 64, 64  panel 4
    // 256, 0 -> 128, 64 panel 5
    void MapVisibleToMatrix(int matrix_width, int matrix_height,
                            int visible_x, int visible_y,
                            int *matrix_x, int *matrix_y) const
    {
        // Panel 1 and 2 are fine
        *matrix_y = visible_y;
        *matrix_x = visible_x;
        // Panel 3 and 4 are one row up
        if (visible_x >= 128 && visible_x <= 255)
        {
            *matrix_x = visible_x - 128;
            *matrix_y = visible_y + 64;
        }
        // Panel 5 is two rows up one block over
        else if (visible_x >= 256 && visible_x <= 319)
        {
            *matrix_x = visible_x - 192;
            *matrix_y = visible_y + 128;
        }
    }
};

#endif /* PIXEL_MAPPER_H */

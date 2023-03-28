#include "draw.hpp"

#include "led-matrix.h"

#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <signal.h>

using namespace rgb_matrix;


void draw(SDL_Surface *surface)
{
    RGBMatrix::Options defaults;
    RuntimeOptions runtime_opt;

    defaults.hardware_mapping = "regular"; // or e.g. "adafruit-hat"
    defaults.panel_type = "FM6126A";
    defaults.rows = 64;
    defaults.cols = 64;
    defaults.chain_length = 2;
    defaults.parallel = 3;
    defaults.show_refresh_rate = true;
    defaults.pwm_lsb_nanoseconds = 50;

    // turn off hardware pulse
    defaults.hardware_pulse = false;

    runtime_opt.gpio_slowdown = 4;
    Canvas *canvas = CreateMatrixFromOptions(defaults, runtime_opt);
    if (canvas == NULL)
        return;

    // Set every pixel in canvas based on framebuffer, size is 64x64
    canvas->Fill(0, 100, 0);

    Uint8* pixelData = new Uint8[WIDTH * HEIGHT * 4];
    memcpy(pixelData, surface->pixels, WIDTH * HEIGHT * 4);

    for (int x = 0; x < WIDTH; x++)
    {
        for (int y = 0; y < HEIGHT; y++)
        {
            int index = (x + y * WIDTH) * 4;
            canvas->SetPixel(x, y, pixelData[index], pixelData[index + 1], pixelData[index + 2]);
        }
    }

    delete[] pixelData;

    sleep(100000000);


    // Animation finished. Shut down the RGB matrix.
    canvas->Clear();
    delete canvas;
}
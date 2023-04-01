#include "graphics/draw.hpp"
#include "graphics/rpi/pixel_mapper.hpp"

#include "led-matrix.h"

#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <signal.h>

using namespace rgb_matrix;

RGBMatrix *canvas;
FrameCanvas *off_screen_canvas_;
Uint8 *pixelData;

void init()
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
    // defaults.disable_hardware_pulsing = true;

    runtime_opt.gpio_slowdown = 4;
    canvas = CreateMatrixFromOptions(defaults, runtime_opt);
    if (canvas == NULL)
        return;

    canvas->ApplyPixelMapper(new PixelMapper());

    off_screen_canvas_ = canvas->CreateFrameCanvas();
    pixelData = new Uint8[WIDTH * HEIGHT * 4];
}

void draw(SDL_Surface *surface)
{
    // Set every pixel in canvas based on framebuffer, size is 64x64
    SDL_LockSurface(surface);
    memcpy(pixelData, surface->pixels, WIDTH * HEIGHT * 4);
    SDL_UnlockSurface(surface);

    off_screen_canvas_->Fill(0, 0, 0);


    for (int x = 0; x < WIDTH; x++)
    {
        for (int y = 0; y < HEIGHT; y++)
        {
            int index = (x + y * WIDTH) * 4;
            off_screen_canvas_->SetPixel(x, y, pixelData[index], pixelData[index + 1], pixelData[index + 2]);
        }
    }

    off_screen_canvas_ = canvas->SwapOnVSync(off_screen_canvas_);

    // delete[] pixelData;

    // sleep(100000000);

    // Animation finished. Shut down the RGB matrix.
    // canvas->Clear();
    // delete canvas;
}
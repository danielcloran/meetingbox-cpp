#include "graphics/renderer.hpp"
#include "graphics/rpi/pixel_mapper.hpp"


#include "led-matrix.h"

#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <signal.h>
#include <array>
#include <algorithm>


using namespace rgb_matrix;

RGBMatrix *canvas;
FrameCanvas *off_screen_canvas_;
std::array<Uint8, WIDTH * HEIGHT * 4> pixelData;
std::array<Uint8, WIDTH * HEIGHT * 4> priorPixelData;
std::array<Uint8, WIDTH * HEIGHT * 4> maskPixelData;

void Renderer::initialize()
{
    RGBMatrix::Options defaults;
    RuntimeOptions runtime_opt;

    defaults.hardware_mapping = "regular";
    defaults.panel_type = "FM6126A";
    defaults.rows = 64;
    defaults.cols = 64;
    defaults.chain_length = 2;
    defaults.parallel = 3;
    defaults.show_refresh_rate = true;
    defaults.pwm_lsb_nanoseconds = 50;
    defaults.led_rgb_sequence = "BGR";

    runtime_opt.gpio_slowdown = 4;

    canvas = CreateMatrixFromOptions(defaults, runtime_opt);
    if (canvas == NULL) {
        std::cerr << "Failed to create RGBMatrix canvas" << std::endl;
        return;
    }
    canvas->ApplyPixelMapper(new CustomMapper());

    off_screen_canvas_ = canvas->CreateFrameCanvas();
}

void Renderer::draw(SDL_Surface *surface)
{
    // copy pixelDatat to priorPixelData
    std::copy(pixelData.begin(), pixelData.end(), priorPixelData.begin());
    // Set every pixel in canvas based on framebuffer, size is 64x64
    SDL_LockSurface(surface);
    std::copy((Uint8 *)surface->pixels,(Uint8 *)surface->pixels + WIDTH * HEIGHT * 4, pixelData.begin());
    SDL_UnlockSurface(surface);

    // mask the different pixels to pixelMask
    std::transform(pixelData.begin(), pixelData.end(), priorPixelData.begin(), maskPixelData.begin(), [](Uint8 a, Uint8 b) { return a - b; });

    // off_screen_canvas_->Clear();

    for (int x = 0; x < WIDTH; x++)
    {
        for (int y = 0; y < HEIGHT; y++)
        {
            int index = (x + y * WIDTH) * 4;
            off_screen_canvas_->SetPixel(x, y, maskPixelData[index], maskPixelData[index + 1], maskPixelData[index + 2]);
        }
    }

    off_screen_canvas_ = canvas->SwapOnVSync(off_screen_canvas_);
}

void Renderer::quit() {
    delete canvas;
}
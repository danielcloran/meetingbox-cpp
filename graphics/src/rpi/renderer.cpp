#include "graphics/renderer.hpp"
#include "graphics/rpi/pixel_mapper.hpp"

#include "led-matrix.h"
#include "graphics.h"

#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <signal.h>
#include <array>
#include <algorithm>

using namespace rgb_matrix;

RGBMatrix *canvas;
FrameCanvas *off_screen_canvas_;
std::array<std::array<uint32_t, WIDTH * HEIGHT>, 2> pixelData;
// std::array<Uint8, WIDTH * HEIGHT * 4> priorPixelData;
std::array<uint32_t, WIDTH * HEIGHT> maskPixelData;

uint8_t get_red(uint32_t color)
{
    return color & 0xFF;
}

uint8_t get_green(uint32_t color)
{
    return (color >> 8) & 0xFF;
}

uint8_t get_blue(uint32_t color)
{
    return (color >> 16) & 0xFF;
}

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
    if (canvas == NULL)
    {
        std::cerr << "Failed to create RGBMatrix canvas" << std::endl;
        return;
    }
    canvas->ApplyPixelMapper(new CustomMapper());

    off_screen_canvas_ = canvas->CreateFrameCanvas();

    // Set every pixel in the two buffers to black
    std::fill(pixelData[0].begin(), pixelData[0].end(), 0);
    std::fill(pixelData[1].begin(), pixelData[1].end(), 0);
}

// bool operator!=(const rgb_matrix::Color& a, const rgb_matrix::Color& b) {
//     return a.r != b.r || a.g != b.g || a.b != b.b;
// }

// rgb_matrix::Color get_canvas_pixel(Canvas *canvas, int x, int y) {
//     uint8_t r, g, b;
//     canvas->GetPixel(x, y, &r, &g, &b);
//     return Color(r, g, b);
// }

// void Renderer::draw(SDL_Surface *surface)
// {
//     static bool currentBuffer = 0;
//     // Swap buffers
//     currentBuffer = !currentBuffer;

//     // Set every pixel in canvas based on framebuffer, size is 64x64
//     SDL_LockSurface(surface);
//     std::copy((uint32_t *)surface->pixels, (uint32_t *)surface->pixels + (WIDTH * HEIGHT), pixelData[currentBuffer].begin());
//     SDL_UnlockSurface(surface);

//     for (int x = 0; x < WIDTH; x++)
//     {
//         for (int y = 0; y < HEIGHT; y++)
//         {
//             int index = (x + y * WIDTH);
//             Color current_pixel = Color(get_red(pixelData[currentBuffer][index]), get_green(pixelData[currentBuffer][index]), get_blue(pixelData[currentBuffer][index]));
//             Color offscreen_pixel = get_canvas_pixel(off_screen_canvas_, x, y);

//             if (current_pixel != offscreen_pixel)
//             {
//                 off_screen_canvas_->SetPixel(x, y, current_pixel.r, current_pixel.g, current_pixel.b);
//             }
//         }
//     }

//     off_screen_canvas_ = canvas->SwapOnVSync(off_screen_canvas_);
//     // print offscreen canvas pointer
//     std::cout << off_screen_canvas_ << std::endl;
// }

void Renderer::draw(SDL_Surface *surface)
{
    static bool currentBuffer = 0;
    // Swap buffers
    currentBuffer = !currentBuffer;

    // std::cout << "Current buffer: " << currentBuffer << std::endl;
    // Set every pixel in canvas based on framebuffer, size is 64x64
    SDL_LockSurface(surface);
    std::copy((uint32_t *)surface->pixels, (uint32_t *)surface->pixels + (WIDTH * HEIGHT), pixelData[currentBuffer].begin());
    SDL_UnlockSurface(surface);
    // uint8_t *pixels = (uint8_t *)surface->pixels;

    // off_screen_canvas_->Clear();

    for (int x = 0; x < WIDTH; x++)
    {
        for (int y = 0; y < HEIGHT; y++)
        {
            int index = (x + y * WIDTH);
            if (pixelData[currentBuffer][index] != pixelData[!currentBuffer][index])
            {
                canvas->SetPixel(x, y, get_red(pixelData[currentBuffer][index]), get_green(pixelData[currentBuffer][index]), get_blue(pixelData[currentBuffer][index]));
                std::cout << "Pixel changed (x,y):" << x << "," << y << std::endl;
            }
            // int index = (x + y * WIDTH) * 4;

            // canvas->SetPixel(x, y, pixels[index], pixels[index + 1], pixels[index + 2]);
        }
    }
    // canvas->
    // off_screen_canvas_ = canvas->SwapOnVSync(off_screen_canvas_);
    // std::cout << off_screen_canvas_ << std::endl;
}

void Renderer::quit()
{
    delete canvas;
}
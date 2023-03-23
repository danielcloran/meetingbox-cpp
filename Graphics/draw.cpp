#include "draw.hpp"

#include "led-matrix.h"

#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <signal.h>

using namespace rgb_matrix;


void draw()
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


    runtime_opt.gpio_slowdown = 4;
    Canvas *canvas = CreateMatrixFromOptions(defaults, runtime_opt);
    if (canvas == NULL)
        return;

    // Slowly fading between colors randomly
    int red = 0;
    int green = 0;
    int blue = 0;
    while (true) {
        red = rand() % 255;
        green = rand() % 255;
        blue = rand() % 255;
        for (int i = 0; i < 255; i++) {
            canvas->Fill(red, green, blue);
            usleep(10);
        }
    }


    // const sf::Texture& texture = renderTexture.getTexture();
    // const sf::Uint8* pixels = texture.copyToImage().getPixelsPtr();
    // std::vector<unsigned char> framebuffer(pixels, pixels + width * height * 4);

    // // Set every pixel in canvas based on framebuffer, size is 64x64
    // canvas->Fill(0, 100, 0);
    // for (int x = 0; x < width; x++)
    // {
    //     for (int y = 0; y < height; y++)
    //     {
    //         int index = (x + y * width) * 4;
    //         canvas->SetPixel(x, y, framebuffer[index], framebuffer[index + 1], framebuffer[index + 2]);
    //     }
    // }

    sleep(100000000);


    // Animation finished. Shut down the RGB matrix.
    // canvas->Clear();
    // delete canvas;
}
#include "draw.hpp"

#include "led-matrix.h"

#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <signal.h>

void init_window(sf::RenderWindow &window) {
    window.create(sf::VideoMode(1, 1), "My Window");
    window.setVisible(false);
}


void draw(sf::RenderWindow &, sf::RenderTexture &renderTexture, int width, int height)
{
    RGBMatrix::Options defaults;
    defaults.hardware_mapping = "regular"; // or e.g. "adafruit-hat"
    defaults.rows = 64;
    defaults.chain_length = 1;
    defaults.parallel = 1;
    defaults.show_refresh_rate = true;
    Canvas *canvas = RGBMatrix::CreateFromFlags(&argc, &argv, &defaults);
    if (canvas == NULL)
        return 1;

    canvas->SetPixl(0, 0, 255, 0, 0);
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
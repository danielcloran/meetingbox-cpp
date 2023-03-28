#include <SDL2/SDL.h>
#include <iostream>
#include "draw.hpp"

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_EVERYTHING & ~SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Surface *surface = SDL_CreateRGBSurface(0, WIDTH, HEIGHT, 32, 0,0,0,0);
    if (!surface)
    {
        std::cerr << "Failed to create surface: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    init();

    // SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0, 100, 0));

    // SDL_Rect square = {100, 7, 50, 50};
    // SDL_FillRect(surface, &square, SDL_MapRGB(surface->format, 80, 0, 0));

    // // Do something with pixelData, e.g., send it to the LED panel using SPI
    // // Draw surface to window

    // draw(surface);

    bool quit = false;
    SDL_Event event;
    float angle = 0.0f;

    while (!quit)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = true;
            }
        }

        SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0, 100, 0));

        int squareWidth = 20;
        int squareHeight = 20;
        int centerX = WIDTH / 2;
        int centerY = HEIGHT / 2;
        int offsetX = squareWidth / 2;
        int offsetY = squareHeight / 2;

        float radians = angle * M_PI / 180.0f;
        int xOffset = static_cast<int>(offsetX * cos(radians) - offsetY * sin(radians));
        int yOffset = static_cast<int>(offsetX * sin(radians) + offsetY * cos(radians));

        SDL_Rect square = {centerX - xOffset, centerY - yOffset, squareWidth, squareHeight};
        SDL_FillRect(surface, &square, SDL_MapRGB(surface->format, 80, 0, 0));

        draw(surface);

        // angle += 1.0f;
        // if (angle >= 360.0f)
        // {
        //     angle = 0.0f;
        // }

        SDL_Delay(20);
    }

    SDL_FreeSurface(surface);
    SDL_Quit();

    return 0;
}
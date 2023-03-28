#include <SDL2/SDL.h>
#include <iostream>
#include "draw.hpp"


int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_EVERYTHING & ~SDL_INIT_VIDEO) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Surface* surface = SDL_CreateRGBSurface(0, WIDTH, HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    if (!surface) {
        std::cerr << "Failed to create surface: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0, 100, 0));

    SDL_Rect square = {100, 20, 50, 50};
    SDL_FillRect(surface, &square, SDL_MapRGB(surface->format, 80, 0, 0));

    // Do something with pixelData, e.g., send it to the LED panel using SPI
    // Draw surface to window

    draw(surface);

    SDL_FreeSurface(surface);
    SDL_Quit();

    return 0;
}
#include "graphics/draw.hpp"

SDL_Window* window;
void init() {
    window = SDL_CreateWindow("SDL2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        return;
    }
}

void draw(SDL_Surface *surface)
{
    SDL_Surface* windowSurface = SDL_GetWindowSurface(window);
    SDL_BlitSurface(surface, NULL, windowSurface, NULL);
    SDL_UpdateWindowSurface(window);
}
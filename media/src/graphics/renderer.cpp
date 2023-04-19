#include "graphics/renderer.hpp"

#define AUGMENTED_WIDTH 1600
#define AUGMENTED_HEIGHT 320

SDL_Window* window;
SDL_Surface* windowSurface;

void Renderer::initialize() {
    window = SDL_CreateWindow("SDL2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, AUGMENTED_WIDTH, AUGMENTED_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        return;
    }
    windowSurface = SDL_GetWindowSurface(window);
}

void Renderer::draw(SDL_Surface *surface)
{
    SDL_UpperBlitScaled(surface, NULL, windowSurface, NULL);
    SDL_UpdateWindowSurface(window);
}

void Renderer::quit() {
    SDL_DestroyWindow(window);
}

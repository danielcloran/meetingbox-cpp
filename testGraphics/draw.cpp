#include "draw.hpp"


void draw(SDL_Surface *surface)
{
    SDL_Window* window = SDL_CreateWindow("SDL2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(surface);
        SDL_Quit();
        return;
    }
    // draw
    SDL_Surface* windowSurface = SDL_GetWindowSurface(window);
    SDL_BlitSurface(surface, NULL, windowSurface, NULL);
    SDL_UpdateWindowSurface(window);
    // wait
    SDL_Event event;
    bool quit = false;
    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }
    }
}
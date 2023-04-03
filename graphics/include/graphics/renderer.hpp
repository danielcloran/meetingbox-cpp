#ifndef RENDERER_hpp
#define RENDERER_hpp

#include <iostream>
#include <SDL2/SDL.h>

class Renderer
{
public:
    static void initialize();
    static void draw(SDL_Surface *surface);
    static void quit();
};

#endif // RENDERER_hpp

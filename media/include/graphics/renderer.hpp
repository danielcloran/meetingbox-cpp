#ifndef RENDERER_hpp
#define RENDERER_hpp

#include <iostream>
#include <SDL2/SDL.h>

const int WIDTH = 320;
const int HEIGHT = 64;

class Renderer
{
public:
    static void initialize();
    static void draw(SDL_Surface *surface);
    static void quit();
};

#endif // RENDERER_hpp

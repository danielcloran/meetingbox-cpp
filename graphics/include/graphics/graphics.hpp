#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include <vector>
#include <map>
#include <unordered_map>
#include <iostream>
#include <thread>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "graphics/renderer.hpp"

class Screen
{
public:
    enum ScreenType
    {
        ALL,
        SIDES,
        TOP,
        TOP_BAR, // 64x12
        FIRST_SIDE
        // BOTTOM_BAR,
        // TOP_RIGHT_CORNER, // 32x32
        // TOP_LEFT_CORNER,
        // BOTTOM_RIGHT_CORNER,
        // BOTTOM_LEFT_CORNER
    };

    static const std::unordered_map<ScreenType, SDL_Rect> screen_sizes_;
};

const std::unordered_map<Screen::ScreenType, SDL_Rect> Screen::screen_sizes_ = {
    {ScreenType::ALL, {0, 0, 320, 64}},
    {ScreenType::SIDES, {0, 0, 256, 64}},
    {ScreenType::TOP, {256, 0, 64, 64}},
    {ScreenType::FIRST_SIDE, {0, 0, 64, 64}}};

struct ProcessScreen
{
    SDL_Surface *surface;
    Screen::ScreenType screen_type;
};

class Graphics
{
public:
    void initialize();
    void start();
    void quit();

    static Graphics &instance()
    {
        static Graphics s;
        return s;
    }

private:
    SDL_Surface *screen_;
    SDL_Renderer *renderer_;

    std::map<int, ProcessScreen> process_screens_;

    volatile bool quit_;
    std::thread paint_thread_;
};

#endif // GRAPHICS_HPP

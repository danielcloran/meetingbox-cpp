#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include <vector>
#include <array>
#include <map>
#include <unordered_map>

#include <iostream>
#include <thread>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "graphics/renderer.hpp"
#include "events/event_queue.hpp"

class Screen
{
public:
    enum ScreenType
    {
        ALL,
        MIMICK_ALL, // one 64x64 screen that gets copied to all panels
        SIDES,
        MIMICK_SIDES, // one 64x64 screen that gets copied to all sides
        TOP,
        TOP_BAR,
        BOTTOM_BAR
    };

    static const std::unordered_map<ScreenType, SDL_Rect> screen_sizes_;
    static const std::vector<SDL_Rect> sides_;
    static const SDL_Rect top_;
};

const std::vector<SDL_Rect> Screen::sides_ = {
    {0, 0, 64, 64},
    {64, 0, 64, 64},
    {128, 0, 64, 64},
    {192, 0, 64, 64}};
const SDL_Rect Screen::top_ = {256, 0, 64, 64};

const std::unordered_map<Screen::ScreenType, SDL_Rect> Screen::screen_sizes_ = {
    {ScreenType::ALL, {0, 0, 320, 64}},
    {ScreenType::MIMICK_ALL, {0, 0, 64, 64}},
    {ScreenType::SIDES, {0, 0, 256, 64}},
    {ScreenType::MIMICK_SIDES, {0, 0, 64, 64}},
    {ScreenType::TOP, {256, 0, 64, 64}},
    {ScreenType::TOP_BAR, {0, 0, 320, 12}},
    {ScreenType::BOTTOM_BAR, {0, 52, 320, 12}}};
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
    int add_process_screen(Screen::ScreenType screen_type);
    void remove_process_screen(int screen_id);
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

    int process_screen_id_;
};

#endif // GRAPHICS_HPP

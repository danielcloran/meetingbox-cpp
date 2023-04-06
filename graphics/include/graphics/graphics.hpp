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

// Define TRANSPARENT as a  Uint32 color
#define TRANSPARENT 0x00000000
// #define TRANSPARENT SDL_Color(0, 0, 0, 128)
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

    static std::unordered_map<ScreenType, SDL_Rect> screen_sizes_;
    static const std::vector<SDL_Rect> sides_;
    static const SDL_Rect top_;
};

const std::vector<SDL_Rect> Screen::sides_ = {
    {0, 0, 64, 64},
    {64, 0, 64, 64},
    {128, 0, 64, 64},
    {192, 0, 64, 64}};
const SDL_Rect Screen::top_ = {256, 0, 64, 64};

std::unordered_map<Screen::ScreenType, SDL_Rect> Screen::screen_sizes_ = {
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
    SDL_Renderer *renderer;
    Screen::ScreenType screen_type;
};

namespace graphics
{
    extern SDL_Renderer *renderer_;

    void initialize();
    void loop();
    void quit();

    int add_process_screen(Screen::ScreenType screen_type);
    void remove_process_screen(int screen_id);
    SDL_Renderer *get_process_renderer(int screen_id);

    namespace internal
    {
        extern SDL_Surface *screen_;
        extern volatile bool quit_;

        extern int process_screen_id_;
        extern std::map<int, ProcessScreen> process_screens_;
    }
}

#endif // GRAPHICS_HPP

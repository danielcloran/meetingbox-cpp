#include "processes/screensaver_process.hpp"

ScreensaverProcess::ScreensaverProcess(int processId, int screenId, Json::Value info) : Process(processId, screenId, info) {
    defaultSaver = ScreensaverTypes::LOADING;
}

static void SolidColor(SDL_Renderer *renderer, SDL_Rect size)
{
    SDL_SetRenderDrawColor(renderer, 0, 220, 0, 255);
    SDL_RenderClear(renderer);
}

static void RotatingColors(SDL_Renderer *renderer, SDL_Rect size, long long timeElapsed)
{
    static int continuum = 0;
    continuum %= 3 * 255;
    int r = 0, g = 0, b = 0;
    if (continuum <= 255)
    {
        int c = continuum;
        b = 255 - c;
        r = c;
    }
    else if (continuum > 255 && continuum <= 511)
    {
        int c = continuum - 256;
        r = 255 - c;
        g = c;
    }
    else
    {
        int c = continuum - 512;
        g = 255 - c;
        b = c;
    }
    SDL_SetRenderDrawColor(renderer, r, g, b, 150);
    SDL_RenderClear(renderer);
    continuum += 1;
}

static void Loading(SDL_Renderer *renderer, SDL_Rect size, long long timeElapsed) {
    static IMG_Animation *gif = IMG_LoadAnimation("../graphics/resizeflow.gif");
    static int currentFrame = 0;
        // play gif
    currentFrame = (currentFrame + 1) % gif->count;
    // make tex from frame
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, gif->frames[currentFrame]);
    SDL_RenderCopy(renderer, tex, NULL, NULL);
}

void ScreensaverProcess::draw(SDL_Renderer *renderer, SDL_Rect size, long long timeElapsed)
{
    switch (defaultSaver)
    {
    case ScreensaverTypes::SOLID_COLOR:
        SolidColor(renderer, size);
        break;
    case ScreensaverTypes::ROTATING_COLORS:
        RotatingColors(renderer, size, timeElapsed);
        break;
    case ScreensaverTypes::GAME_OF_LIFE:
        // GameOfLife(windowCanvas);
        break;
    case ScreensaverTypes::LOADING:
        Loading(renderer, size, timeElapsed);
        break;
    case ScreensaverTypes::THE_OFFICE:
        // TheOffice(windowCanvas);
        break;
    default:
        SolidColor(renderer, size);
    };
}
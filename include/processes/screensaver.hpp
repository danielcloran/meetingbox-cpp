#ifndef SCREENSAVER_hpp
#define SCREENSAVER_hpp

#include <stdio.h>
#include "processes/process.hpp"

enum ScreensaverTypes
{
    SOLID_COLOR,
    ROTATING_COLORS,
    GAME_OF_LIFE,
    LOADING,
    THE_OFFICE
};

class ScreensaverProcess : public Process
{
public:
    ScreensaverProcess(int processId, int screenId, Json::Value info);
    const ProcessType type = ProcessType::SCREENSAVER;

private:
    void draw(SDL_Renderer *renderer, SDL_Rect size, long long timeElapsed) override;
    ScreensaverTypes defaultSaver = ScreensaverTypes::SOLID_COLOR;
};

#endif /* SCREENSAVER_hpp */

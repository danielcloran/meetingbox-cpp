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
    virtual ~ScreensaverProcess() = default;
    const ProcessType type = ProcessType::SCREENSAVER;

private:
    void draw(long long timeElapsed) override;
    ScreensaverTypes defaultSaver = ScreensaverTypes::SOLID_COLOR;
};

#endif /* SCREENSAVER_hpp */

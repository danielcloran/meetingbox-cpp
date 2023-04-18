#ifndef SCREENSAVER_PROCESS_hpp
#define SCREENSAVER_PROCESS_hpp

#include <stdio.h>
#include "processes/process.hpp"

class HotwordDetectedProcess : public Process
{
public:
    HotwordDetectedProcess(int processId, int screenId, Json::Value info);

private:
    void draw(SDL_Renderer *renderer, SDL_Rect size, long long timeElapsed) override;

    enum State
    {
        LISTENING,
        PROCESSESING,
        IDLE
    };
    State state = State::LISTENING;
};

#endif /* SCREENSAVER_PROCESS_hpp */

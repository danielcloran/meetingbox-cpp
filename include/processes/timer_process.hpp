#ifndef TIMER_PROCESS_hpp
#define TIMER_PROCESS_hpp

#include <stdio.h>
#include "processes/process.hpp"

class TimerProcess : public Process
{
public:
    TimerProcess(int processId, int screenId, Json::Value info);
    const ProcessType type = ProcessType::TIMER;

private:
    int timerInitialDuration;
    int timeRemaining;
    float timerPercentage;
    int clearRow;

    // void DrawFull(WindowCanvas *windowCanvas);
    // void DrawTimerBlock(WindowCanvas *windowCanvas);

    void handleTimeElapsedEvent(events::EventPointer theEvent);

    void draw(SDL_Renderer *renderer, SDL_Rect size, long long timeElapsed) override;

    void listenForEvents();
};

#endif /* TIMER_PROCESS_hpp */

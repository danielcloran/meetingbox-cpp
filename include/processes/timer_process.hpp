#ifndef TIMERPROCESS_hpp
#define TIMERPROCESS_hpp

#include <stdio.h>
#include "processes/process.hpp"

class TimerProcess : public Process
{
public:
    const ProcessType type = ProcessType::TIMER;

private:
    int timerInitialDuration;
    int timeRemaining;
    float timerPercentage;
    int clearRow;

    // void DrawFull(WindowCanvas *windowCanvas);
    // void DrawTimerBlock(WindowCanvas *windowCanvas);

    void handleTimeElapsedEvent(events::EventPointer theEvent);

    void configure(Json::Value info) override;
    void draw(SDL_Renderer *renderer, long long timeElapsed) override;

    void listenForEvents();
};

#endif // TIMERPROCESS_hpp

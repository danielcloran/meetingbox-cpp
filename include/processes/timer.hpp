#ifndef TIMER_PROCESS_hpp
#define TIMER_PROCESS_hpp

#include <stdio.h>
#include "processes/process.hpp"

class TimerProcess : public Process
{
public:
    TimerProcess(int processId, int screenId, Json::Value info);
    virtual ~TimerProcess() = default;
    const ProcessType type = ProcessType::TIMER;

    int total_ms() const { return timerInitialDuration; }
    int remaining_ms() const { return timeRemaining; }
    float remaining_fraction() const { return timerPercentage; }

private:
    int timerInitialDuration;
    int timeRemaining;
    float timerPercentage;
    int clearRow;

    void draw(long long timeElapsed) override;
};

#endif /* TIMER_PROCESS_hpp */

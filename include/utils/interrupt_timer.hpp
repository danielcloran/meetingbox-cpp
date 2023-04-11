#ifndef INTERRUPT_TIMER_HPP
#define INTERRUPT_TIMER_HPP

#include <stdio.h>
#include "events/event_queue.hpp"

class InterruptTimer
{
public:
    InterruptTimer();
    void update();
    void setFrameRate(int frameRate);
    ~InterruptTimer();

private:
    long long current_timestamp();
    long long lastUpdateMilliseconds;
    long long millisecondsBetweenUpdates;

    volatile bool stop;
    events::ScopedRemover listeners;
    void handleStopEvent(events::EventPointer theEvent);
};

#endif // INTERRUPT_TIMER_HPP

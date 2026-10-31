//
//  InterruptTimer.cpp
//  TimeBox_Xcode
//
//  Created by Daniel Cloran on 12/17/21.
//

#include "utils/interrupt_timer.hpp"

#include <unistd.h>
#include <sys/time.h>
#include <iostream>

InterruptTimer::InterruptTimer() : listeners(events::queue)
{
    stop = false;
    setFrameRate(120); // Default
    lastUpdateMilliseconds = current_timestamp();

    listeners.appendListener(EventType::stop, std::bind(&InterruptTimer::handleStopEvent, this, std::placeholders::_1));
    listeners.appendListener(EventType::set_frame_rate, std::bind(&InterruptTimer::handleSetFrameRateEvent, this, std::placeholders::_1));
}

void InterruptTimer::handleStopEvent(events::EventPointer theEvent)
{
    stop = true;
}

void InterruptTimer::handleSetFrameRateEvent(events::EventPointer theEvent)
{
    auto *event = static_cast<SetFrameRateEvent *>(theEvent.get());
    if (event)
    {
        setFrameRate(event->getFrameRate());
    }
}

void InterruptTimer::update()
{
    while (!stop)
    {
        long long timeElapsed = current_timestamp() - lastUpdateMilliseconds;
        long long timePassed;
        if (timeElapsed > millisecondsBetweenUpdates)
        {
            std::cout << "Time Elapsed (Thread is falling behind): " << timeElapsed << std::endl;
            timePassed = timeElapsed;
        }
        else
        {
            usleep(1000 * useconds_t(millisecondsBetweenUpdates - timeElapsed)); // usleep is in microseconds
            timePassed = millisecondsBetweenUpdates;
        }

        tick(timePassed);
    }
}

void InterruptTimer::tick(long long timePassed)
{
    if (timePassed < 0)
        timePassed = 0;
    lastUpdateMilliseconds = current_timestamp();
    events::queue.enqueue(std::make_shared<TimeElapsedEvent>(timePassed));
    events::queue.enqueue(std::make_shared<DrawEvent>(timePassed));
}

void InterruptTimer::setFrameRate(int frameRate)
{
    if (frameRate <= 0)
    {
        std::cerr << "[InterruptTimer] Ignoring invalid frame rate: " << frameRate << std::endl;
        return;
    }

    millisecondsBetweenUpdates = 1000 / frameRate;
}

long long InterruptTimer::current_timestamp()
{
    struct timeval te;
    gettimeofday(&te, NULL);                                         // get current time
    long long milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000; // calculate milliseconds
    return milliseconds;
}

InterruptTimer::~InterruptTimer() {}
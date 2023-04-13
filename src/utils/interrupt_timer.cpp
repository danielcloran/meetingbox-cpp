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
}

void InterruptTimer::handleStopEvent(events::EventPointer theEvent)
{
    stop = true;
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

        lastUpdateMilliseconds = current_timestamp();
        // std::cout <<"outputting time elapsed event" << std::endl;
        events::queue.enqueue(std::make_shared<TimeElapsedEvent>(timeElapsed));
        events::queue.enqueue(std::make_shared<Draw>(timeElapsed));
    }
}

void InterruptTimer::setFrameRate(int frameRate)
{
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
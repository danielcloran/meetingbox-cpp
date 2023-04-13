//
//  TimerProcess.cpp
//  TimeBox_Xcode
//
//  Created by Daniel Cloran on 12/22/21.
//

#include "processes/timer_process.hpp"
#include <iostream>
#include <unistd.h>
#include <regex>

#include "json/json.h"

// PARSE EVENT
static int match_duration(const std::string &input, const std::regex &re)
{
    std::smatch match;
    std::regex_search(input, match, re);
    if (match.empty())
    {
        // std::cout << "Time pattern does NOT match" << std::endl;
        return 0;
    }

    std::vector<double> vec = {0, 0, 0, 0, 0, 0}; // years, months, days, hours, minutes, seconds

    for (size_t i = 1; i < match.size(); ++i)
    {

        if (match[i].matched)
        {
            std::string str = match[i];
            str.pop_back(); // remove last character.
            vec[i - 1] = std::stod(str);
        }
    }

    int duration = 31556926 * vec[0] +   // years
                   2629743.83 * vec[1] + // months
                   86400 * vec[2] +      // days
                   3600 * vec[3] +       // hours
                   60 * vec[4] +         // minutes
                   1 * vec[5];           // seconds

    if (duration == 0)
    {
        std::cout << "Not valid input" << std::endl;
        return 0;
    }

    return duration;
}

void TimerProcess::configure(Json::Value info)
{
    int seconds = 0;
    std::string timeToParse = info["time"].asString();
    // std::cout << "Time: " << timeToParse << std::endl;

    std::regex rshort("^((?!T).)*$");
    if (std::regex_match(timeToParse, rshort)) // no T (Time) exist
    {
        std::regex r("P([[:d:]]+Y)?([[:d:]]+M)?([[:d:]]+D)?");
        seconds = match_duration(timeToParse, r);
    }
    else
    {
        std::regex r("P([[:d:]]+Y)?([[:d:]]+M)?([[:d:]]+D)?T([[:d:]]+H)?([[:d:]]+M)?([[:d:]]+S|[[:d:]]+\\.[[:d:]]+S)?");
        seconds = match_duration(timeToParse, r);
    }

    timerInitialDuration = seconds * 1000;
    timeRemaining = timerInitialDuration;

    timerPercentage = 0;
    clearRow = 0;
}

void TimerProcess::draw(SDL_Renderer *renderer, long long timeElapsed)
{
    int fakeBrightness = 255;
    int numLeds = (windowCanvas->height() + 1) * (windowCanvas->height() + 1); // TODO: MATH needs some work here

    // std::cout << timeRemaining << std::endl;

    if (timerPercentage <= 1)
    {
        // Straight Down Line Timer
        int whatRow = int((numLeds * timerPercentage) / (windowCanvas->height() + 1));
        float topRowPercentage = 1 - (((numLeds * timerPercentage) / (windowCanvas->height() + 1)) - whatRow);

        fakeBrightness = int(255 * topRowPercentage); // TODO have brightness be a setting

        windowCanvas->DrawLine(0, whatRow, windowCanvas->width(), whatRow, Color(0, 0, fakeBrightness));

        for (int otherRowsY = whatRow + 1; otherRowsY < windowCanvas->height() + 1; otherRowsY++)
        {
            windowCanvas->DrawLine(0, otherRowsY, windowCanvas->width(), otherRowsY, Color(0, 0, 255));
        }
        windowCanvas->DrawLine(0, whatRow - 1, windowCanvas->width(), whatRow - 1, Color(0, 0, 0));
    }
}







// TimerProcess::TimerProcess(int id, int seconds) : Process(id, "timer") {
//     timerInitialDuration = seconds * 1000;

//     listenForEvents();
//     scopedRemover->addLi
//     eventQueue->publish(EventType::add_to_screen_manager, new AddToScreenManagerEvent(id, processType, WindowCanvasTypes::CORE));
// }

// void TimerProcess::listenForEvents()
// {
//     // These callbacks must all access member variables accessed by other threads safely
//     EventListeners listeners{
//         {EventType::time_elapsed, eventCallback(&TimerProcess::handleTimeElapsedEvent)}
//     };
//     eventQueue->addListeners(listeners, &scopedRemover);
// }

// void TimerProcess::handleTimeElapsedEvent(EventObjPointer theEvent)
// {
//     assert(theEvent->getType() == EventType::time_elapsed);
//     TimeElapsedEvent *timeElapsedEvent = dynamic_cast<TimeElapsedEvent *>(theEvent.get());

//     boost::lock_guard<boost::mutex> lock{mutex};
//     timeRemaining += -(timeElapsedEvent->getTimeElapsed());
//     timerPercentage = 1 - (timeRemaining / float(timerInitialDuration));

//     if (timerPercentage > 1)
//     {
//         deleteMe();
//     }
// }

// void TimerProcess::SetInvisible()
// {
//     visible = false;
// }

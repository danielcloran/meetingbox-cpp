//
//  TimerProcess.cpp
//  TimeBox_Xcode
//
//  Created by Daniel Cloran on 12/22/21.
//

#include "processes/timer.hpp"

#include "graphics/timebox_timer.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

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

TimerProcess::TimerProcess(int processId, int screenId, Json::Value info) : Process(processId, screenId, info)
{
    std::cout << "Configuring Timer Process" << std::endl;
    int seconds = 0;
    if (info["time"].isInt())
    {
        seconds = info["time"].asInt();
    }
    else if (info["time"].isString())
    {
        std::string timeToParse = info["time"].asString();
        const bool digits_only = !timeToParse.empty() &&
            std::all_of(timeToParse.begin(), timeToParse.end(),
                        [](unsigned char ch) { return std::isdigit(ch) != 0; });
        if (digits_only)
        {
            seconds = std::stoi(timeToParse);
        }
        else
        {
            std::regex rshort("^((?!T).)*$");
            if (std::regex_match(timeToParse, rshort)) // no T (Time) exist
            {
                std::regex r("P([0-9]+Y)?([0-9]+M)?([0-9]+D)?");
                seconds = match_duration(timeToParse, r);
            }
            else
            {
                std::regex r("P([0-9]+Y)?([0-9]+M)?([0-9]+D)?T([0-9]+H)?([0-9]+M)?([0-9]+(?:\\.[0-9]+)?S)?");
                seconds = match_duration(timeToParse, r);
            }
        }
    }

    std::cout << "Seconds: " << seconds << std::endl;

    timerInitialDuration = seconds * 1000;
    timeRemaining = timerInitialDuration;

    timerPercentage = timerInitialDuration > 0 ? 1.f : 0.f;
    clearRow = 0;
}

void TimerProcess::draw(long long timeElapsed)
{
    if (timerInitialDuration > 0 && timeElapsed > 0)
    {
        if (timeElapsed >= static_cast<long long>(timeRemaining))
            timeRemaining = 0;
        else
            timeRemaining -= static_cast<int>(timeElapsed);
    }

    if (timerInitialDuration > 0)
        timerPercentage = timeRemaining / float(timerInitialDuration);
    else
        timerPercentage = 0.f;

    if (timerPercentage < 0.0f)
        timerPercentage = 0.0f;
    if (timerPercentage > 1.0f)
        timerPercentage = 1.0f;

    // Same field the original SDL draw filled: one blue level per row.
    if (graphics::context_)
        graphics::paint_timebox_timer(*graphics::context_, timeRemaining, timerInitialDuration);

    if (timerInitialDuration > 0 && timeRemaining <= 0 && timeElapsed > 0)
        deleteMe();
}

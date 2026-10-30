#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include "events/event_queue.hpp"
#include "graphics/graphics.hpp"

using ProcessInfo = std::map<std::string, std::string>;

enum ProcessType
{
    NONE,
    TIMER,
    SCREENSAVER,
    HOTWORD_DETECTED,
    STOCK_TICKER,
    SCREEN_NUMS,
    TALKER_TRACKER,
    IDENTIFY
};

class Process
{
public:
    const ProcessType type = ProcessType::NONE; // must be set by child class
    Process(int processId, int screenId, Json::Value info) : listeners(events::queue)
    {
        id = processId;
        this->screenId = screenId;
        if (info["name"] != Json::Value::null)
            name = info["name"].asString();
        else
            name = "Unnamed Process";

        listeners.appendListener(EventType::draw, std::bind(&Process::handleDrawEvent, this, std::placeholders::_1));
    }
    ~Process(){};

    bool visible = true;
    std::string name;
    int screenId;

private:
    void handleDrawEvent(events::EventPointer theEvent)
    {
        if (!visible || deleted)
            return;

        const DrawEvent *drawEvent = static_cast<const DrawEvent *>(theEvent.get());
        draw(drawEvent->getTimeElapsed());
    }
    virtual void draw(long long timeElapsed)
    {
        std::cout << "Error, Draw calling in Base Class Process";
    };

protected:
    int id;
    events::ScopedRemover listeners;

    bool deleted = false;
    void deleteMe()
    {
        if (deleted)
            return;

        deleted = true;
        listeners.reset();
        events::queue.enqueue(std::make_shared<DeleteProcessEvent>(id));
    }
};

#endif /* PROCESS_HPP */

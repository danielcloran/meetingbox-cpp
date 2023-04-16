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
        screenId = screenId;
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

        ProcessScreen screen = graphics::get_process_screen(screenId);
        if (screen.renderer == nullptr)
        {
            std::cerr << "Error, screen is null on process " << id << " with name" << name << std::endl;
            return;
        }
        const DrawEvent *drawEvent = static_cast<const DrawEvent *>(theEvent.get());
        draw(screen.renderer, Screen::screen_sizes_.at(screen.screen_type), drawEvent->getTimeElapsed());
    }
    virtual void draw(SDL_Renderer *renderer, SDL_Rect size, long long timeElapsed)
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

//
//  Process.hpp
//  TimeBox_Xcode
//
//  Created by Daniel Cloran on 12/22/21.
//

#ifndef Process_hpp
#define Process_hpp

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
    SCREEN_SAVER,
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
        name = info["name"].asString();

        listeners.appendListener(EventType::draw, std::bind(&Process::handleDrawEvent, this, std::placeholders::_1));
    }
    ~Process()
    {
        if (visible)
        {
            events::queue.enqueue(std::make_unique<RemoveFromScreenEvent>(id));
        }
        events::queue.enqueue(std::make_unique<DeleteProcessEvent>(id));
    };

    bool visible = true;
    std::string name;

private:
    void handleDrawEvent(events::EventPointer theEvent)
    {
        if (!visible)
            return;

        SDL_Renderer *renderer = graphics::get_process_renderer(screenId);
        if (renderer == NULL)
        {
            std::cerr << "Error, renderer is null on process " << id << " with name" << name << std::endl;
            return;
        }
        const DrawEvent *drawEvent = static_cast<const DrawEvent *>(theEvent.get());
        draw(renderer, drawEvent->getTimeElapsed());
    }
    virtual void draw(SDL_Renderer *renderer, long long timeElapsed)
    {
        std::cout << "Error, Draw calling in Base Class Process";
    };

    virtual void configure(Json::Value info)
    {
        std::cout << "Error, configure calling in Base Class Process";
    };

protected:
    int id;
    int screenId;
    events::ScopedRemover listeners;
};

#endif /* Process_hpp */

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
// include jsoncpp
// #include "json/json.h"

// #include <boost/thread.hpp>
// #include <boost/chrono.hpp>

using ProcessInfo = std::map<std::string, std::string>;

enum ProcessTypes
{
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
    Process(int givenId, std::string givenName) : scopedRemover(events::queue)
    {
        name = givenName;
        id = givenId;
    }
    ~Process()
    {
        if (visible)
        {
            events::queue.enqueue(std::make_unique<RemoveFromScreenEvent>(id));
        }
    };
    static std::shared_ptr<Process> getInstance(int id, std::string intent, ProcessInfo info)
    {
        std::logic_error("getInstance called on base class... no class meets intent string");
        return nullptr;
    };

    static void updateInstance(std::string intent, ProcessInfo info)
    {
        std::logic_error("updateInstance called on base class... no class meets intent string");
    };

    // void Render(SDL_Renderer* renderer) {
    //     if (visible) {
    //         Draw(renderer);
    //     }
    // }

    virtual void Draw()
    {
        std::cout << "Error, Draw calling in Base Class Process";
    };

    bool visible = true;
    std::string name;
    int processType;

protected:
    int id;
    Screen::ScreenType location;

    // TODO: alreadyDeleted is a hack to prevent double deletes
    bool alreadyDeleted;
    void deleteMe()
    {
        if (!alreadyDeleted)
        {
            events::queue.enqueue(std::make_unique<DeleteProcessEvent>(id));
            alreadyDeleted = true;
        }
    }

    // boost::mutex mutex;

    // void handleToggleVisibilityEvent(const events::EventPointer &event)
    // {
    //     const ToggleVisibilityEvent * changedEvent = static_cast<const ChangedEvent *>(event.get());
    //     visible = !visible;
    // }

    events::ScopedRemover scopedRemover;
};

#endif /* Process_hpp */

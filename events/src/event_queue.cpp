#include "events/event_queue.hpp"

void EventManager::initialize()
{
    quit_ = false;
    event_thread_ = std::thread(&EventManager::event_loop, this);
}

void EventManager::event_loop()
{
    while (!queue.waitFor(std::chrono::milliseconds(10)) && !quit_)
    {
        queue.process();
    }
};

void EventManager::quit()
{
    quit_ = true;
    event_thread_.join();
};

#ifndef EVENT_QUEUE_HPP
#define EVENT_QUEUE_HPP

#include <stdio.h>
#include <thread>

#include <eventpp/eventqueue.h>
#include <eventpp/utilities/scopedremover.h>

#include "events/events.hpp"


namespace events
{
    using EventPointer = std::shared_ptr<Event>;
    struct EventPolicy
    {
        static EventType getEvent(const EventPointer &event)
        {
            return event->getType();
        }
    };
    using EQ = eventpp::EventQueue<EventType, void(const EventPointer &), EventPolicy>;
    using ScopedRemover = eventpp::ScopedRemover<EQ>;

    extern EQ queue;

    void initialize();
    void quit();

    namespace internal
    {
        void event_loop();
        extern std::atomic<bool> quit_;
        extern std::thread event_thread_;
    }
}

#endif /* EVENT_QUEUE_HPP */
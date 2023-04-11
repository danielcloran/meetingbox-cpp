#ifndef EVENT_QUEUE_HPP
#define EVENT_QUEUE_HPP

#include <stdio.h>
#include <thread>

#include <eventpp/eventqueue.h>
#include <eventpp/utilities/scopedremover.h>

#include "events/events.hpp"

#include <thread>

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

// class EventManager
// {
// public:
//     static EventManager &instance()
//     {
//         static EventManager s;
//         return s;
//     }

//     void initialize();
//     void quit();

// private:
//     void event_loop();

//     volatile bool quit_;
//     std::thread event_thread_;
// };
// #define eventCallback(func) (std::bind(func, this, std::placeholders::_1))

// // Global Event Queue Format Init
// using EventppQueue = eventpp::EventQueue<int, void(EventObjPointer)>;
// using ScopedRemover = eventpp::ScopedRemover<EventppQueue>;
// using EventppCallback = EventppQueue::Callback;
// inline EventppQueue eventppQueue;
// using EventListeners = std::map<const int, EventppCallback>;

// class EventQueue
// {
// public:
//     EventQueue();
//     void run();
//     void stop();
//     void addListener(EventType type, EventppCallback callback, ScopedRemover *scopedRemover);
//     void addListeners(EventListeners listeners, ScopedRemover *scopedRemover);
//     void publish(EventType type, Event *theEvent);
//     ~EventQueue(){};

// private:
//     void handleStopEvent(EventObjPointer theEvent);
//     ScopedRemover scopedRemover;
//     volatile bool _stop;
// };

// // declare global eventQueue for all to use!
// extern EventQueue *eventQueue;

#endif /* EVENT_QUEUE_HPP */
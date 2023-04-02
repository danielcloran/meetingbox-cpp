// //
// //  EventQueue.cpp
// //  TimeBox_Xcode
// //
// //  Created by Daniel Cloran on 12/17/21.
// //

// #include "events/event_queue.hpp"
// #include <time.h>
// #include <iostream>

// EventQueue* eventQueue = new EventQueue();

// EventQueue::EventQueue() : scopedRemover(eventppQueue)
// {
//     _stop = false;

//     EventListeners listeners{
//         {EventType::stop, eventCallback(&EventQueue::handleStopEvent)},
//     };
//     eventQueue->addListeners(listeners, &scopedRemover);
// };

// void EventQueue::handleStopEvent(EventObjPointer theEvent)
// {
//     _stop = true;
// }

// void EventQueue::run()
// {
// //    clock_t start, end;
// //    double cpu_time_used;
//     while (!_stop)
//     {
//         eventppQueue.wait();
// //        start = clock();
//         eventppQueue.process();
// //        end = clock();
// //        std::cout << "processing: " << ((double) (end - start)) / CLOCKS_PER_SEC << std::endl;
//     }
// }

// void EventQueue::addListeners(EventListeners listeners, ScopedRemover* scopedRemover)
// {
//     for (const auto &listener : listeners)
//     {
//         scopedRemover->appendListener(listener.first, listener.second);
//     }
// }

// void EventQueue::addListener(EventType type, EventppCallback callback, ScopedRemover* scopedRemover)
// {
//     scopedRemover->appendListener(type, callback);
// }

// void EventQueue::publish(EventType type, Event *theEvent)
// {
// //    if (type != 11) {
// //        std::cout << "publishing: " << type << std::endl;
// //    }
//     eventppQueue.enqueue(type, std::unique_ptr<Event>(theEvent));
// }

#include "events/event_queue.hpp"
#include <iostream>
namespace events
{
    EQ queue;
    namespace internal
    {
        std::atomic<bool> quit_(false);
        std::thread event_thread_;

        void event_loop()
        {
            for (;;)
            {
                while (!queue.waitFor(std::chrono::milliseconds(10)) && !quit_.load());
                if (quit_.load())
                {
                    break;
                }

                queue.process();
            }
        }
    }

    void initialize()
    {
        internal::event_thread_ = std::thread(internal::event_loop);
    }

    void quit()
    {
        internal::quit_.store(true);
        if (internal::event_thread_.joinable())
        {
            internal::event_thread_.join();
        }
    }
}
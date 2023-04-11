#include "events/event_queue.hpp"
#include <iostream>
namespace events
{
    EQ queue;
    namespace detail
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
        detail::event_thread_ = std::thread(detail::event_loop);
    }

    void quit()
    {
        detail::quit_.store(true);
        if (detail::event_thread_.joinable())
        {
            detail::event_thread_.join();
        }
    }
}
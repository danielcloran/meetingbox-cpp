#include "events/event_queue.hpp"

namespace events
{
    namespace detail
    {
        volatile bool quit_ = false;
        std::thread event_thread_;

        void event_loop()
        {
            while (!queue.waitFor(std::chrono::milliseconds(10)) && !quit_)
            {
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
        detail::quit_ = true;
        if (detail::event_thread_.joinable())
        {
            detail::event_thread_.join();
        }
    }
}
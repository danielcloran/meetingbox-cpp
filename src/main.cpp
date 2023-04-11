#include <iostream>
#include <thread>
#include <chrono>

#include "graphics/graphics.hpp"
#include "events/event_queue.hpp"

#include "utils/interrupt_timer.hpp"

void interruptHandler(int dummy)
{
    std::cout << "\nStopping Program" << std::endl;
    graphics::quit();
}

int main()
{
    signal(SIGINT, interruptHandler);

    events::initialize();

    std::thread timerThread(&InterruptTimer::update, new InterruptTimer());
    graphics::initialize();

    graphics::loop(); // blocking
    events::queue.dispatch(std::make_unique<StopEvent>());

    timerThread.join();
    events::quit();

    return 0;
}
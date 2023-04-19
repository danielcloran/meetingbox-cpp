#include <iostream>
#include <thread>
#include <chrono>

#include "graphics/graphics.hpp"
#include "events/event_queue.hpp"
#include "audio/audio.hpp"

#include "processes/process_manager.hpp"

#include "utils/interrupt_timer.hpp"

void interruptHandler(int dummy)
{
    std::cout << "\nStopping Program" << std::endl;
    graphics::quit();
}

int main()
{
    signal(SIGINT, interruptHandler);

    if (SDL_Init(SDL_INIT_EVERYTHING & ~SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
    }

    events::initialize();
    audio::initialize();

    std::thread timerThread(&InterruptTimer::update, new InterruptTimer());

    graphics::initialize();
    std::unique_ptr<ProcessManager> processManager = std::make_unique<ProcessManager>();
    graphics::loop(); // blocking

    events::queue.dispatch(std::make_unique<StopEvent>());

    timerThread.join();
    audio::quit();
    events::quit();

    return 0;
}
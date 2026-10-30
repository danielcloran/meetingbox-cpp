#include <iostream>
#include <thread>
#include <chrono>
#include <signal.h>

#include "graphics/graphics.hpp"
#include "events/event_queue.hpp"
#include "audio/audio.hpp"

#include "processes/process_manager.hpp"
#include "demo/timer_demo.hpp"

#include "utils/interrupt_timer.hpp"

#include <string>

void interruptHandler(int dummy)
{
    std::cout << "\nStopping Program" << std::endl;
    graphics::quit();
}

int main(int argc, char **argv)
{
    bool timer_demo = false;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--demo=timer" || arg == "--no-audio")
            timer_demo = true;
    }
    if (timer_demo) {
        std::cout << "MeetingBox timer demo (--no-audio, no hotword)" << std::endl;
        return run_timer_demo(argc, argv);
    }

    signal(SIGINT, interruptHandler);

    std::cout << "Starting MeetingBox..." << std::endl;

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
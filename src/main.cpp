#include <iostream>
#include <thread>
#include <chrono>

#include "graphics/graphics.hpp"
#include "events/event_queue.hpp"

void interruptHandler(int dummy)
{
    std::cout << "\nStopping Program" << std::endl;
    graphics::quit();
}

int main()
{
    signal(SIGINT, interruptHandler);


    events::initialize();

    graphics::initialize();
    graphics::loop(); // blocking

    events::quit();

    // Start the process manager
    // ProcessManager::start();

    // Set the flag to exit the task thread
    // should_exit.store(true);
    // event_thread.join();
    //   std::atomic<bool> should_exit(false);
    // std::thread event_thread(event_loop, std::ref(should_exit));

    // queue.appendListener(EventType::stop, [](const EventPointer &event)
    //                      { std::cout << "Received stop!" << std::endl; });
    // // Wait for the task thread to finish

    // queue.dispatch(std::make_unique<StopEvent>());

    // Stop the process manager
    // ProcessManager::stop();

    // // Shutdown the graphics subsystem
    // Graphics::shutdown();

    // // Stop the event loop
    // EventLoop::stop();

    // // Shutdown the audio subsystem
    // Audio::shutdown();

    return 0;
}
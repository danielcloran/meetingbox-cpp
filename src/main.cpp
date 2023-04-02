#include <iostream>
#include <thread>
#include <chrono>

#include "graphics/graphics.hpp"
#include "events/event_queue.hpp"

void event_loop(std::atomic<bool> &should_exit)
{
    while (!queue.waitFor(std::chrono::milliseconds(10)) && !should_exit.load())
    {
        queue.process();
    }
}

int main()
{
    // Graphics::initialize();

    // Start the process manager
    // ProcessManager::start();

    std::atomic<bool> should_exit(false);
    std::thread event_thread(event_loop, std::ref(should_exit));

    run();

    // Wait for the user to press a key to exit the program
    std::cout << "Press any key to exit...\n";
    std::cin.get();

    // Set the flag to exit the task thread
    should_exit.store(true);
    event_thread.join();

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
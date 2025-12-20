#include "processes/hotword_detected.hpp"

HotwordDetectedProcess::HotwordDetectedProcess(int processId, int screenId, Json::Value info) : Process(processId, screenId, info)
{
    state = State::LISTENING;
    listeners.appendListener(EventType::hotword_detected, [this](events::EventPointer e)
                             {
                                    std::cout << "HotwordDetectedProcess: " << state << std::endl;
                                state = State::LISTENING; });
    listeners.appendListener(EventType::silence_detected, [this](events::EventPointer e)
                             { state = State::PROCESSESING; });
    listeners.appendListener(EventType::intent_received, [this](events::EventPointer e)
                             { state = State::IDLE; });
}

void HotwordDetectedProcess::draw(long long timeElapsed)
{
    switch (state)
    {
    case State::LISTENING:
        graphics::display_message("Listening...");
        break;
    case State::PROCESSESING:
        graphics::display_message("Processing...");
        break;
    case State::IDLE:
    default:
        graphics::display_message("Idle");
        break;
    };
}
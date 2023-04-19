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

void HotwordDetectedProcess::draw(SDL_Renderer *renderer, SDL_Rect size, long long timeElapsed)
{
    switch (state)
    {
    case State::LISTENING:
        // draw 12 dots in a circle around the center of the screen
        SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
        // SDL_RenderClear(renderer);
        SDL_RenderDrawLine(renderer, 0, 31, 63, 31);

        SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
        for (int i = 0; i < 12; i++)
        {
            int x = size.w / 2 + 26 * cos(i * 2 * M_PI / 12);
            int y = size.h / 2 + 26 * sin(i * 2 * M_PI / 12);
            SDL_RenderDrawPoint(renderer, x - 1, y - 1);
        }

        break;
    case State::PROCESSESING:
        // RotatingColors(renderer, size, timeElapsed);
        break;
    case State::IDLE:
    default:
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        break;
    };
}
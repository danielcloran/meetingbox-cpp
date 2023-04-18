#include <iostream>
#include <memory>
#include <unordered_map>
#include <functional>
#include "json/json.h"

#include "processes/process.hpp"
#include "processes/timer.hpp"
#include "processes/hotword_detected.hpp"
#include "processes/screensaver.hpp"

template <typename T>
std::function<std::shared_ptr<T>(int, int, Json::Value)> createProcess() {
    return [](int processId, int screenId, Json::Value info) {
        return std::make_shared<T>(processId, screenId, info);
    };
}

std::unordered_map<ProcessType, std::function<std::shared_ptr<Process>(int, int, Json::Value)>> process_factories = {
    {ProcessType::TIMER, createProcess<TimerProcess>()},
    {ProcessType::SCREENSAVER, createProcess<ScreensaverProcess>()},
    {ProcessType::HOTWORD_DETECTED, createProcess<HotwordDetectedProcess>()}
    // Add other process factories here
};

std::shared_ptr<Process> create_process(ProcessType type, int processId, int screenId, Json::Value info)
{
    auto it = process_factories.find(type);
    if (it != process_factories.end())
    {
        return it->second(processId, screenId, info);
    }
    return nullptr;
}

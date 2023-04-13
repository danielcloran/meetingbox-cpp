#include <iostream>
#include <memory>
#include <unordered_map>
#include <functional>
#include "json/json.h"

#include "processes/process.hpp"
#include "processes/timer_process.hpp"

using ProcessFactory = std::function<std::unique_ptr<Process>(int, int, Json::Value)>;

// Register process factories
std::unordered_map<ProcessType, ProcessFactory> process_factories = {
    {ProcessType::TIMER, [](int processId, int screenId, Json::Value info) { return std::make_unique<TimerProcess>(processId, screenId, info); }}
    // Add other process factories here
};

std::unique_ptr<Process> create_process(ProcessType type, int processId, int screenId, Json::Value info)
{
    auto it = process_factories.find(type);
    if (it != process_factories.end())
    {
        return it->second(processId, screenId, info);
    }
    return nullptr;
}

#ifndef PROCESS_MANAGER_hpp
#define PROCESS_MANAGER_hpp

#include <stdio.h>
#include <stdlib.h>
#include <list>
#include "events/event_queue.hpp"
#include "graphics/graphics.hpp"
#include "process.hpp"

#include "json/json.h"

// #include "Process.hpp"
// #include "NetworkManager.hpp"
// #include "Processes.hpp"
// #include "EventQueue.hpp"
// #include "ScreenManager.hpp"


class ProcessManager {
public:
    ProcessManager();
    // std::shared_ptr<Process> getProcessById(int id);
    ~ProcessManager();
private:
    int processId;
    // ScreenManager * screenManager;
    std::map<int, std::shared_ptr<Process>> processList;

    events::ScopedRemover listeners;

    void handleCreateProcessEvent(events::EventPointer theEvent);
    // void handleEditProcessEvent(events::EventPointer theEvent);
    // void handleDeleteProcessEvent(events::EventPointer theEvent);

    // void handleToggleProcessVisibilityEvent(events::EventPointer theEvent);

    // void socketGetProcessList(std::string event, Json::Value data);
    // void socketEditProcessList(std::string event, Json::Value data);
};

#endif /* PROCESS_MANAGER_hpp */
//
//  ProcessManager.cpp
//  TimeBox_Xcode
//
//  Created by Daniel Cloran on 12/22/21.
//

#include "processes/process_manager.hpp"
#include "processes/process_factory.hpp"
#include <iostream>

ProcessManager::ProcessManager() : listeners(events::queue)
{
    processId = 1;

    // screenManager = new ScreenManager(this);

    listeners.appendListener(EventType::create_process, std::bind(&ProcessManager::handleCreateProcessEvent, this, std::placeholders::_1));
    listeners.appendListener(EventType::edit_process, std::bind(&ProcessManager::handleEditProcessEvent, this, std::placeholders::_1));
    listeners.appendListener(EventType::delete_process, std::bind(&ProcessManager::handleDeleteProcessEvent, this, std::placeholders::_1));


    // Initially Add A ScreenSaver;
//    eventQueue->publish(EventType::create_process, new CreateProcessEvent("showTimer", {{"time", "30"}}));
    // events::queue.enqueue(std::make_shared<CreateProcessEvent>(ProcessType::SCREEN_SAVER, Json::Value()));
    // create timer with time in info
    Json::Value timerInfo;
    timerInfo["time"] = 5;
    events::queue.enqueue(std::make_shared<CreateProcessEvent>(ProcessType::SCREENSAVER, timerInfo));


    // events::queue.enqueue(std::make_shared<TimeElapsedEvent>(timeElapsed));

    // eventQueue->publish(EventType::socket_subscribe, new SocketSubscribeEvent(string("get_processes_" + string(BOX_ID)), std::bind(&ProcessManager::socketGetProcessList, this, _1, _2)));
    // eventQueue->publish(EventType::socket_subscribe, new SocketSubscribeEvent(string("edit_processes_" + string(BOX_ID)), std::bind(&ProcessManager::socketEditProcessList, this, _1, _2)));
}

// void ProcessManager::listenForEvents()
// {
//     EventListeners listeners{
//         {EventType::create_process, eventCallback(&ProcessManager::handleCreateProcessEvent)},
//         {EventType::edit_process, eventCallback(&ProcessManager::handleEditProcessEvent)},
//         {EventType::delete_process, eventCallback(&ProcessManager::handleDeleteProcessEvent)},
//         {EventType::toggle_visibility, eventCallback(&ProcessManager::handleToggleProcessVisibilityEvent)},
//     };
//     eventQueue->addListeners(listeners, &scopedRemover);
// }

// void ProcessManager::socketGetProcessList(std::string event, json::value data) {
//     std::cout << "Launching event  " << std::endl;
//     json::value process_update;
//     process_update["tester"] = json::value::string("me");
//     eventQueue->publish(EventType::socket_emit, new SocketEmitEvent("testing", process_update));
// }

// void ProcessManager::socketEditProcessList(std::string event, json::value data) {}



void ProcessManager::handleCreateProcessEvent(events::EventPointer theEvent) {
    assert(theEvent->getType() == EventType::create_process);
    const CreateProcessEvent * createEvent = static_cast<const CreateProcessEvent *>(theEvent.get());

    ProcessType processType = static_cast<ProcessType>(createEvent->getProcessType());
    Json::Value info = createEvent->getInfo();

    int screenId = graphics::add_process_screen(Screen::MIMICK_SIDES);
    std::shared_ptr<Process> newProcess = create_process(processType, processId, screenId, info);

    processes[processId++] = newProcess;
}

void ProcessManager::handleEditProcessEvent(events::EventPointer theEvent) {};

void ProcessManager::handleDeleteProcessEvent(events::EventPointer theEvent) {
    assert(theEvent->getType() == EventType::delete_process);
    const DeleteProcessEvent * deleteEvent = static_cast<const DeleteProcessEvent *>(theEvent.get());

    int id = deleteEvent->getId();
    graphics::remove_process_screen(processes[id]->screenId);
    processes.erase(id);

    std::cout << "Deleted process: " << id << std::endl;
};

// void ProcessManager::handleToggleProcessVisibilityEvent(EventObjPointer theEvent) {
//     assert(theEvent->getType() == EventType::toggle_visibility);
//     ToggleVisibilityEvent *toggleVisibilityEvent = dynamic_cast<ToggleVisibilityEvent *>(theEvent.get());
//     int id = toggleVisibilityEvent->getProcessId();
//     int isVisible = toggleVisibilityEvent->getIsVisible();
//     std::cout <<"setting process: " << id << " to: " << isVisible << std::endl;

//     auto it = processList.find(id);
//     if (it == processList.end()) {
//         std::cout << "Error: attempted to toggle nonexistance process visibility" << std::endl;
//     } else {
//         it->second->visible = isVisible;
//     }
// }

// void ProcessManager::beginRendering() {
//     std::cout << "Beginning Render" << std::endl << std::endl << std::endl << std::endl;
//     screenManager->beginRendering();
// }

// std::shared_ptr<Process> ProcessManager::getProcessById(int id) {
//     auto it = processList.find(id);
//     if (it == processList.end()) {
//         return nullptr;
//     }
//     return it->second;
// }


ProcessManager::~ProcessManager() {};

//
//  Events.h
//  TimeBox_Xcode
//
//  Created by Daniel Cloran on 12/17/21.
//

#ifndef EVENTS_H
#define EVENTS_H

#include <eventpp/utilities/eventmaker.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

enum class EventType
{
    create_process,
    edit_process,
    delete_process,
    packet_received,
    hotword_detected,
    lex_process_finished,
    time_elapsed,
    stop,
    toggle_visibility,
    add_to_screen,
    remove_from_screen,
    audio_frame_received,
    socket_subscribe,
    socket_unsubscribe,
    socket_emit,
    draw
};

class Event
{
public:
    explicit Event(const EventType type) : type(type) {}
    virtual ~Event() {}

    EventType getType() const
    {
        return type;
    }

private:
    EventType type;
};

// System Processes
EVENTPP_MAKE_EMPTY_EVENT(LexProcessFinishedEvent, Event, EventType::lex_process_finished);
// EVENTPP_MAKE_EMPTY_EVENT(PacketReceivedEvent, Event, EventType::packet_received);

using ProcessInfo = std::map<std::string, std::string>;
EVENTPP_MAKE_EVENT(CreateProcessEvent, Event, EventType::create_process, (std::string, getIntent, setIntent), (ProcessInfo, getInfo, setInfo));
EVENTPP_MAKE_EVENT(EditProcessEvent, Event, EventType::edit_process, (std::string, getIntent, setIntent));
EVENTPP_MAKE_EVENT(DeleteProcessEvent, Event, EventType::delete_process, (int, getId, setId));

EVENTPP_MAKE_EVENT(ToggleVisibilityEvent, Event, EventType::toggle_visibility, (int, getProcessId, setProcessId), (int, getIsVisible, setIsVisible));
EVENTPP_MAKE_EVENT(AddToScreenEvent, Event, EventType::add_to_screen, (int, getProcessId, setProcessId), (int, getProcessType, setProcessType), (int, getCanvasWindowType, setCanvasWindowType));
EVENTPP_MAKE_EVENT(RemoveFromScreenEvent, Event, EventType::remove_from_screen, (int, getProcessId, setProcessId));

EVENTPP_MAKE_EVENT(AudioFrameReceivedEvent, Event, EventType::audio_frame_received, (int, getId, setId));

struct SSTSource
{
    int ID;
    float activity;
    float x;
    float y;
    float z;
};

EVENTPP_MAKE_EVENT(TimeElapsedEvent, Event, EventType::time_elapsed, (long long, getTimeElapsed));
EVENTPP_MAKE_EVENT(DrawEvent, Event, EventType::draw, (long long, getTimeElapsed));
EVENTPP_MAKE_EMPTY_EVENT(HotwordDetectedEvent, Event, EventType::hotword_detected);

EVENTPP_MAKE_EMPTY_EVENT(StopEvent, Event, EventType::stop);

#endif /* EVENTS_H */

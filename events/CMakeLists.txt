message(STATUS "Configuring events subsystem")

# Set the sources for the events subsystem
set(EVENTS_SOURCES
    src/event_queue.cpp
)

add_library(events ${EVENTS_SOURCES})

target_link_libraries(events PUBLIC ${SHARED_LIBS})
target_include_directories(events PUBLIC ${VCPKG_INCLUDE} "include")

message(STATUS "Configuring audio subsystem")

add_library(audio src/audio.cpp)

# Set the sources for the audio subsystem
if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64")
    message(STATUS "Building on a Raspberry Pi")
    target_link_libraries(audio PRIVATE ${CMAKE_SOURCE_DIR}/libs/porcupine/lib/raspberry-pi/cortex-a72-aarch64/libpv_porcupine.so)
else()
    message(STATUS "Not building on a Raspberry Pi")
    target_link_libraries(audio PRIVATE ${CMAKE_SOURCE_DIR}/libs/porcupine/lib/mac/x86_64/libpv_porcupine.dylib)
endif()


target_link_libraries(audio PUBLIC ${SHARED_LIBS} SDL2::SDL2 events)
target_include_directories(audio PUBLIC ${CMAKE_SOURCE_DIR}/libs/porcupine/include)
target_include_directories(audio PUBLIC ${VCPKG_INCLUDE} "include")

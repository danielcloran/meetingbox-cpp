message(STATUS "Configuring graphics subsystem")

add_library(media src/graphics/graphics.cpp src/audio/audio.cpp)

# Set the sources for the events subsystem
if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64")
    message(STATUS "Building on a Raspberry Pi")
    target_sources(media PRIVATE src/graphics/rpi/renderer.cpp)
    target_link_libraries(media PRIVATE ${CMAKE_SOURCE_DIR}/libs/porcupine/lib/raspberry-pi/cortex-a72-aarch64/libpv_porcupine.so)
else()
    message(STATUS "Not building on a Raspberry Pi")
    target_sources(media PRIVATE src/graphics/renderer.cpp)
    target_link_libraries(media PRIVATE ${CMAKE_SOURCE_DIR}/libs/porcupine/lib/mac/x86_64/libpv_porcupine.dylib)
endif()

target_link_libraries(media PRIVATE
    ${SHARED_LIBS}
    events
)
target_link_libraries(media PUBLIC
    $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main> $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

target_include_directories(media PUBLIC "${CMAKE_SOURCE_DIR}/libs/rpi-rgb-led-matrix/include")
target_include_directories(media PUBLIC ${VCPKG_INCLUDE} "include" ${CMAKE_SOURCE_DIR}/libs/porcupine/include)

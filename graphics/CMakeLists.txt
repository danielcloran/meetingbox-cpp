message(STATUS "Configuring graphics subsystem")

add_library(graphics src/graphics.cpp)

# Set the sources for the events subsystem
if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64")
    message(STATUS "Building on a Raspberry Pi")
    target_sources(graphics PRIVATE src/rpi/renderer.cpp)
    target_link_libraries(graphics PRIVATE ${CMAKE_SOURCE_DIR}/libs/rpi-rgb-led-matrix/lib/librgbmatrix.a)
else()
    message(STATUS "Not building on a Raspberry Pi")
    target_sources(graphics PRIVATE src/renderer.cpp)
endif()

target_link_libraries(graphics PUBLIC $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>)
target_link_libraries(graphics PUBLIC ${SHARED_LIBS} SDL2::SDL2 events)
target_include_directories(graphics PUBLIC ${VCPKG_INCLUDE} "include" "${CMAKE_SOURCE_DIR}/libs/rpi-rgb-led-matrix/include")

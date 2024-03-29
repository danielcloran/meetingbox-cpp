cmake_minimum_required(VERSION 3.16)

project(MeetingBox)
set(CMAKE_CXX_STANDARD 17)
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

# Add the main executable
add_executable(main src/main.cpp src/utils/interrupt_timer.cpp src/processes/hotword_detected.cpp src/processes/screensaver.cpp src/processes/timer.cpp src/processes/process_manager.cpp)

# Add vcpkg toolchain file to enable vcpkg packages
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/libs/vcpkg/scripts/buildsystems/vcpkg.cmake)
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${CMAKE_SOURCE_DIR}/libs/arm64-linux/share" "${CMAKE_SOURCE_DIR}/libs/x64-osx/share")
set(VCPKG_INCLUDE "${CMAKE_SOURCE_DIR}/libs/arm64-linux/include/" "${CMAKE_SOURCE_DIR}/libs/x64-osx/include")


# Find the required packages installed by vcpkg
find_package(aws-cpp-sdk-core CONFIG REQUIRED)
find_package(aws-cpp-sdk-lex CONFIG REQUIRED)
find_package(sdl2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)

# Shared Libs
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(eventpp CONFIG REQUIRED)
find_package(jsoncpp CONFIG REQUIRED)
set(SHARED_LIBS fmt::fmt-header-only spdlog::spdlog eventpp::eventpp JsonCpp::JsonCpp)

# Graphics and Audio depend on events
add_subdirectory(events)
add_subdirectory(media)

# Link against the required libraries
target_link_libraries(main PRIVATE
    events
    media
    ${SHARED_LIBS}
)

target_include_directories(main PRIVATE
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/include"
    ${VCPKG_INCLUDE}
)

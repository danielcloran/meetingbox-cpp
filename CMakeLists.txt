cmake_minimum_required(VERSION 3.16)
include(ExternalProject)

project(MeetingBox)
set(CMAKE_CXX_STANDARD 17)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)

# Add vcpkg toolchain file to enable vcpkg packages
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/libs/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")


# Add vcpkg packages to the CMAKE_PREFIX_PATH
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/libs/vcpkg/packages")

# Find the required packages installed by vcpkg
# find_package(aws-cpp-sdk-core CONFIG REQUIRED)
# find_package(aws-cpp-sdk-lex CONFIG REQUIRED)
# find_package(jsoncpp CONFIG REQUIRED)
# find_package(eventpp CONFIG REQUIRED)
find_package(sdl2 CONFIG REQUIRED)

# Add the main executable
add_executable(main main.cpp)

# Check if we're building on a Raspberry Pi
message(${CMAKE_SYSTEM_PROCESSOR})

if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64")
    message(STATUS "Building on a Raspberry Pi")
    target_sources(main PRIVATE ${CMAKE_SOURCE_DIR}/Graphics/draw.cpp)
    target_link_libraries(main PRIVATE ${CMAKE_SOURCE_DIR}/libs/rpi-rgb-led-matrix/lib/librgbmatrix.a)
else()
    message(STATUS "Not building on a Raspberry Pi")
    target_sources(main PRIVATE ${CMAKE_SOURCE_DIR}/testGraphics/draw.cpp)
endif()

# Link against the required libraries
target_link_libraries(main PRIVATE
    # aws-cpp-sdk-core
    # aws-cpp-sdk-lex
    # JsonCpp::JsonCpp
    # eventpp::eventpp
    SDL2::SDL2
)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")



target_include_directories(main PRIVATE
    "${CMAKE_SOURCE_DIR}/"
    "vcpkg_installed/x64-osx/include/SDL2/"
    "libs/vcpkg/packages/"
    "libs/rpi-rgb-led-matrix/include/"
)
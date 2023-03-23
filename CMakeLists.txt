cmake_minimum_required(VERSION 3.16)

project(MeetingBox)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)

# Add vcpkg toolchain file to enable vcpkg packages
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/libs/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")

# Add vcpkg packages to the CMAKE_PREFIX_PATH
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/libs/vcpkg/packages/)

# Find the required packages installed by vcpkg
find_package(aws-cpp-sdk-core CONFIG REQUIRED)
find_package(aws-cpp-sdk-lex CONFIG REQUIRED)
find_package(jsoncpp CONFIG REQUIRED)
find_package(eventpp CONFIG REQUIRED)

#OpenGl
find_package(OpenGL REQUIRED)

# Add the main executable
add_executable(main main.cpp)

# Check if we're building on a Raspberry Pi
if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm" AND ${CMAKE_HOST_SYSTEM_NAME} MATCHES "Linux")
    message(STATUS "Building on a Raspberry Pi")
    target_sources(main PRIVATE "/Graphics/draw.cpp")
    target_link_libraries(my_executable ${CMAKE_SOURCE_DIR}/libs/rpi-rgb-led-matrix/lib/librgbmatrix.a)
else()
    target_sources(main PRIVATE "testGraphics/draw.cpp")
    find_package(glfw3 CONFIG REQUIRED)
    find_package(GLEW REQUIRED)
    target_link_libraries(main PRIVATE glfw GLEW::GLEW)
endif()


# Link against the required libraries
target_link_libraries(main PRIVATE
    aws-cpp-sdk-core
    aws-cpp-sdk-lex
    JsonCpp::JsonCpp
    eventpp::eventpp
    OpenGL::GL
    OpenGL::GLX
)

# Target include
target_include_directories(main PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/
    ${CMAKE_SOURCE_DIR}/libs/
)
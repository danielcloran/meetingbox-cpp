cmake_minimum_required(VERSION 3.16)

project(MeetingBox)
set(CMAKE_CXX_STANDARD 17)
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

# Add the main executable
add_executable(main src/main.cpp)

# Add vcpkg toolchain file to enable vcpkg packages
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/libs/vcpkg/scripts/buildsystems/vcpkg.cmake)
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/libs/vcpkg/packages")
set(VCPKG_INCLUDE "${CMAKE_SOURCE_DIR}/libs/aarch64/include/" "${CMAKE_SOURCE_DIR}/libs/x64-osx/include")


# Find the required packages installed by vcpkg
find_package(aws-cpp-sdk-core CONFIG REQUIRED)
find_package(aws-cpp-sdk-lex CONFIG REQUIRED)
find_package(sdl2 CONFIG REQUIRED)

# Shared Libs
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(eventpp CONFIG REQUIRED)
find_package(jsoncpp CONFIG REQUIRED)
set(SHARED_LIBS fmt::fmt-header-only spdlog::spdlog eventpp::eventpp JsonCpp::JsonCpp)

add_subdirectory(events)
add_subdirectory(graphics)

# Link against the required libraries
target_link_libraries(main PRIVATE
    events
    graphics
    ${SHARED_LIBS}
)

target_include_directories(main PRIVATE
    "${CMAKE_SOURCE_DIR}"
    ${VCPKG_INCLUDE}
)

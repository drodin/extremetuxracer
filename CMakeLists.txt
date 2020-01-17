cmake_minimum_required(VERSION 3.14)

option(CMAKE_VERBOSE_MAKEFILE "Verbose makefile" OFF)

option(HUNTER_KEEP_PACKAGE_SOURCES "Keep third party sources" OFF)
option(HUNTER_STATUS_DEBUG "Print debug info" OFF)
option(HUNTER_BUILD_SHARED_LIBS "Build shared libs" OFF)

include("cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/cpp-pm/hunter/archive/v0.23.244.tar.gz"
    SHA1 "2c0f491fd0b80f7b09e3d21adb97237161ef9835"
)

project(extremetuxracer)

hunter_add_package(SFML)

find_package(SFML COMPONENTS graphics audio CONFIG REQUIRED)
find_package(OpenGL MODULE REQUIRED)

file(GLOB SOURCES src/*.cpp)

add_executable(extremetuxracer ${SOURCES})

set_property(TARGET extremetuxracer PROPERTY CXX_STANDARD 14)

target_compile_definitions(extremetuxracer PRIVATE ETR_DATA_DIR=\".\")

target_include_directories(extremetuxracer PRIVATE ${OPENGL_INCLUDE_DIR})

target_link_libraries(extremetuxracer
    ${OPENGL_LIBRARIES}
    sfml-graphics
    sfml-audio
)

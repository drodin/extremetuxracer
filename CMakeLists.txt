cmake_minimum_required(VERSION 3.14)

# For Android Studio we will force generated toolchain
if(ANDROID_STUDIO)
    set(gen_toolchain "${CMAKE_CURRENT_BINARY_DIR}/generated/toolchain.cmake")
    configure_file(
            "${CMAKE_CURRENT_LIST_DIR}/cmake/template/toolchain.cmake.in"
            "${gen_toolchain}"
            @ONLY
    )
    set(CMAKE_TOOLCHAIN_FILE "${gen_toolchain}" CACHE PATH "" FORCE)
endif()

option(CMAKE_VERBOSE_MAKEFILE "Verbose makefile" OFF)

option(HUNTER_KEEP_PACKAGE_SOURCES "Keep third party sources" OFF)
option(HUNTER_STATUS_DEBUG "Print debug info" OFF)
option(HUNTER_BUILD_SHARED_LIBS "Build shared libs" OFF)

set(HUNTER_ROOT /Users/user/workspace/mobile/hunter)

include("cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/cpp-pm/hunter/archive/v0.23.244.tar.gz"
    SHA1 "2c0f491fd0b80f7b09e3d21adb97237161ef9835"
)

project(extremetuxracer)

hunter_add_package(SFML)

find_package(SFML COMPONENTS graphics audio CONFIG REQUIRED)

if(NOT ANDROID)
    find_package(OpenGL MODULE REQUIRED)
endif()

file(GLOB SOURCES src/*.cpp)

if(ANDROID)
    file(GLOB GL_SOURCES GLES/GL/*.cpp)
    list(APPEND SOURCES ${GL_SOURCES})
    set(OPENGL_INCLUDE_DIR GLES)
endif()

if(NOT ANDROID)
    add_executable(extremetuxracer ${SOURCES})
else()
    add_library(extremetuxracer SHARED ${SOURCES})

    target_compile_definitions(extremetuxracer PRIVATE APPLICATION_ID=\"${APPLICATION_ID}\")
    target_compile_definitions(extremetuxracer PRIVATE VERSION_NAME=\"${VERSION_NAME}\")
    target_compile_definitions(extremetuxracer PRIVATE MOBILE=1)
    target_compile_definitions(extremetuxracer PRIVATE USE_GLES=1)

    set_target_properties(extremetuxracer PROPERTIES DEBUG_POSTFIX "")

    target_link_libraries(extremetuxracer -Wl,--whole-archive sfml-main -Wl,--no-whole-archive)
    target_link_libraries(extremetuxracer android)

    # Add wrapper to preload shared libs
    if(HUNTER_BUILD_SHARED_LIBS)
        find_package(SFML COMPONENTS activity CONFIG REQUIRED)
        target_link_libraries(extremetuxracer sfml-activity)
    endif()
endif()

set_property(TARGET extremetuxracer PROPERTY CXX_STANDARD 14)

target_compile_definitions(extremetuxracer PRIVATE ETR_DATA_DIR=\".\")

target_include_directories(extremetuxracer PRIVATE ${OPENGL_INCLUDE_DIR})

target_link_libraries(extremetuxracer
    ${OPENGL_LIBRARIES}
    sfml-graphics
    sfml-audio
)

if(ANDROID)
    # Synchronize shared libs
    string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
    set(LIBS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/android/app/src/${BUILD_TYPE}/jniLibs/${ANDROID_ABI})

    file(REMOVE_RECURSE ${LIBS_DIR})

    if(HUNTER_BUILD_SHARED_LIBS)
        file(READ ${CMAKE_BINARY_DIR}/_3rdParty/Hunter/install-root-dir HUNTER_INSTALL_DIR)
        file(GLOB SHARED_LIBS ${HUNTER_INSTALL_DIR}/lib/*.so)

        file(COPY ${SHARED_LIBS} DESTINATION ${LIBS_DIR})

        # Hunter buid some shared libs without 'd' debug postfix
        # which is expected by sfml-activity wrapper
        if(${BUILD_TYPE} STREQUAL "debug")
            file(GLOB SHARED_LIBS ${LIBS_DIR}/*.so)
            list(FILTER SHARED_LIBS EXCLUDE REGEX ".*d.so$")

            foreach(SHARED_LIB ${SHARED_LIBS})
                string(REGEX REPLACE ".so$" "d.so" SHARED_LIB_D ${SHARED_LIB})
                file(RENAME ${SHARED_LIB} ${SHARED_LIB_D})
            endforeach()
        endif()
    endif()
endif()

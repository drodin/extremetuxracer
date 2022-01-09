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

option(HUNTER_KEEP_PACKAGE_SOURCES "Keep third party sources" ON)
option(HUNTER_STATUS_DEBUG "Print debug info" OFF)
option(HUNTER_BUILD_SHARED_LIBS "Build shared libs" OFF)

include("cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/drodin/hunter/archive/c870ff6.tar.gz"
    SHA1 "49b146b8607845083fc81ed238f8a4611f6a37d8"
    LOCAL
)

project(extremetuxracer)

hunter_add_package(SFML)

find_package(SFML COMPONENTS graphics audio CONFIG REQUIRED)

if(ANDROID OR IOS)
    hunter_add_package(glu)
    find_package(glu CONFIG REQUIRED)
    set(OPENGL_LIBRARIES glu::GLU)
else()
    find_package(OpenGL MODULE REQUIRED)
endif()

file(GLOB SOURCES src/*.cpp src/*.h)

if(ANDROID)
    add_library(${PROJECT_NAME} SHARED ${SOURCES})

    target_compile_definitions(${PROJECT_NAME} PRIVATE APPLICATION_ID=\"${APPLICATION_ID}\")
    target_compile_definitions(${PROJECT_NAME} PRIVATE VERSION_NAME=\"${VERSION_NAME}\")

    set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "")

    target_link_libraries(${PROJECT_NAME} -Wl,--whole-archive sfml-main -Wl,--no-whole-archive)
    target_link_libraries(${PROJECT_NAME} android)

    # Add wrapper to preload shared libs
    if(HUNTER_BUILD_SHARED_LIBS)
        find_package(SFML COMPONENTS activity CONFIG REQUIRED)
        target_link_libraries(${PROJECT_NAME} sfml-activity)
    endif()
elseif(APPLE)
if(IOS)
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SOURCES} data apple/ios/Assets.xcassets apple/ios/LaunchScreen.storyboard)

    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_LIST_DIR}/cmake/template/iOSBundleInfo.plist.in"
        MACOSX_BUNDLE_BUNDLE_NAME "Tux Racer"
        XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
    )

    set_source_files_properties(apple/ios/Assets.xcassets PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    set_source_files_properties(apple/ios/LaunchScreen.storyboard PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

    target_link_libraries(${PROJECT_NAME} sfml-main)
else()
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SOURCES} data apple/macos/Assets.xcassets apple/macos/extremetuxracer.entitlements)

    set(MACOSX_BUNDLE_APP_CATEGORY "public.app-category.racing-games")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "Extreme Tux Racer"
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_LIST_DIR}/cmake/template/MacOSXBundleInfo.plist.in"
        MACOSX_BUNDLE_COPYRIGHT "Copyright © 2022 Dmitry Rodin. All rights reserved."
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Mac Developer"
        XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "apple/macos/extremetuxracer.entitlements"
        XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES
    )

    set_source_files_properties(apple/macos/Assets.xcassets PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
endif()
    add_definitions(-DGL_SILENCE_DEPRECATION=1)

    set(VERSION_NAME "0.8.106")
    target_compile_definitions(${PROJECT_NAME} PRIVATE VERSION_NAME=\"${VERSION_NAME}\")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        RESOURCE data
        XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon"
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.drodin.${PROJECT_NAME}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${VERSION_NAME}"
        MACOSX_BUNDLE_BUNDLE_VERSION "6"
        XCODE_ATTRIBUTE_INSTALL_PATH "$(LOCAL_APPS_DIR)"
        XCODE_ATTRIBUTE_SKIP_INSTALL "No"
        XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "$ENV{CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM}"
    )
else()
    add_executable(${PROJECT_NAME} ${SOURCES})
endif()

if(ANDROID OR IOS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE MOBILE=1)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_GL4ES=1)
endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)

target_compile_definitions(${PROJECT_NAME} PRIVATE ETR_DATA_DIR=\".\")

target_include_directories(${PROJECT_NAME} PRIVATE ${OPENGL_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME}
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

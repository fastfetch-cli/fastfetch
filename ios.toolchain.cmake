set(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CMAKE_C_COMPILER_WORKS TRUE)
set(CMAKE_CXX_COMPILER_WORKS TRUE)

if(NOT DEFINED ENV{IOS_SDK_PATH} OR "$ENV{IOS_SDK_PATH}" STREQUAL "")
    message(FATAL_ERROR "Please export a SDK 'export IOS_SDK_PATH=/path/to/iPhoneOS16.5.sdk'")
endif()

set(SDK_PATH "$ENV{IOS_SDK_PATH}")
set(CMAKE_SYSROOT ${SDK_PATH})

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

find_program(CMAKE_AR NAMES aarch64-apple-darwin-ar ar)
find_program(CMAKE_RANLIB NAMES aarch64-apple-darwin-ranlib ranlib)
find_program(CMAKE_STRIP NAMES aarch64-apple-darwin-strip strip)
find_program(CMAKE_INSTALL_NAME_TOOL NAMES aarch64-apple-darwin-install_name_tool install_name_tool)

set(FLAGS "-target arm64-apple-ios11.0 -miphoneos-version-min=11.0 -isysroot ${SDK_PATH}")
set(CMAKE_C_FLAGS "${FLAGS}" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "${FLAGS}" CACHE STRING "" FORCE)

set(LINK_FLAGS "-fuse-ld=/usr/local/bin/ld64 -target arm64-apple-ios11.0 -miphoneos-version-min=11.0 -isysroot ${SDK_PATH}")
set(CMAKE_EXE_LINKER_FLAGS "${LINK_FLAGS}" CACHE STRING "" FORCE)
set(CMAKE_SHARED_LINKER_FLAGS "${LINK_FLAGS}" CACHE STRING "" FORCE)

set(CMAKE_FIND_ROOT_PATH ${SDK_PATH})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

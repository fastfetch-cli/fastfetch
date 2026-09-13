set(CMAKE_SYSTEM_NAME Darwin CACHE STRING "" FORCE)
cmake_minimum_required(VERSION 3.16)
project(fastfetch C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(FASTFETCH_PROJECT_VERSION "2.8.0")

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/fastfetch_config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/fastfetch_config.h"
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/fastfetch_datatext.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/fastfetch_datatext.h"
)

option(ENABLE_WAYLAND "Enable Wayland support" OFF)
option(ENABLE_X11 "Enable X11 support" OFF)
option(ENABLE_DBUS "Enable DBus support" OFF)
option(ENABLE_VULKAN "Enable Vulkan support" OFF)
option(BUILD_FLASHFETCH "Build flashfetch executable" OFF)

add_compile_definitions(
    FASTFETCH_TARGET_APPLE=1
    __APPLE__=1
    TARGET_OS_IPHONE=1
)

include_directories(
    src
    src/util
    ${CMAKE_CURRENT_BINARY_DIR}
)

file(GLOB_RECURSE FASTFETCH_SOURCES
    "src/*.c"
    "src/*.cpp"
)

list(FILTER FASTFETCH_SOURCES EXCLUDE REGEX "src/common/windows/.*")
list(FILTER FASTFETCH_SOURCES EXCLUDE REGEX "src/common/haiku/.*")
list(FILTER FASTFETCH_SOURCES EXCLUDE REGEX "src/common/freebsd/.*")
list(FILTER FASTFETCH_SOURCES EXCLUDE REGEX "src/common/sunos/.*")
list(FILTER FASTFETCH_SOURCES EXCLUDE REGEX "src/detection/wayland/.*")
list(FILTER FASTFETCH_SOURCES EXCLUDE REGEX "src/detection/x11/.*")

list(FILTER FASTFETCH_SOURCES EXCLUDE REGEX ".*_linux\\.(c|cpp)$")
list(FILTER FASTFETCH_SOURCES EXCLUDE REGEX ".*_windows\\.(c|cpp)$")
list(FILTER FASTFETCH_SOURCES EXCLUDE REGEX ".*_android\\.(c|cpp)$")
list(FILTER FASTFETCH_SOURCES EXCLUDE REGEX ".*_haiku\\.(c|cpp)$")
list(FILTER FASTFETCH_SOURCES EXCLUDE REGEX ".*_freebsd\\.(c|cpp)$")

list(FILTER FASTFETCH_SOURCES EXCLUDE REGEX "src/common/impl/.*_windows\\.c$")

add_executable(fastfetch ${FASTFETCH_SOURCES})

find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)
find_library(COREFOUNDATION_FRAMEWORK CoreFoundation REQUIRED)
find_library(IOKIT_FRAMEWORK IOKit REQUIRED)
find_library(SYSTEMCONFIGURATION_FRAMEWORK SystemConfiguration REQUIRED)
find_library(COREGRAPHICS_FRAMEWORK CoreGraphics REQUIRED)
find_library(UIKIT_FRAMEWORK UIKit REQUIRED)

target_link_libraries(fastfetch
    PRIVATE
        ${FOUNDATION_FRAMEWORK}
        ${COREFOUNDATION_FRAMEWORK}
        ${IOKIT_FRAMEWORK}
        ${SYSTEMCONFIGURATION_FRAMEWORK}
        ${COREGRAPHICS_FRAMEWORK}
        ${UIKIT_FRAMEWORK}
)

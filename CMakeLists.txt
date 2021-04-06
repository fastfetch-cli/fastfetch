cmake_minimum_required(VERSION 3.0)

project(fastfetch)

option(BUILD_TESTS "Build test programs" OFF)

execute_process(
    COMMAND git rev-list --count HEAD
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_REV_LIST
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_REV_PARSE
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(PROJECT_VERSION "r${GIT_REV_LIST}.${GIT_REV_PARSE}")

if(BUILD_TESTS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -march=native -O3 -pipe -fno-plt")
endif(BUILD_TESTS)

configure_file(src/fastfetch_config.h.in fastfetch_config.h)

find_package (PkgConfig QUIET)

if(${PkgConfig_FOUND})
    pkg_check_modules (GLIB2 QUIET glib-2.0)
endif(${PkgConfig_FOUND})

if(NOT DEFINED GLIB2_INCLUDE_DIRS)
    set(GLIB2_INCLUDE_DIRS
        /usr/include/glib-2.0/
        /usr/lib/glib-2.0/include/
    )
endif(NOT DEFINED GLIB2_INCLUDE_DIRS)

include_directories(
    ${PROJECT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}/src
    ${GLIB2_INCLUDE_DIRS}
)

set(SRCS
    src/common.c
    src/logo.c
    src/util/FFstrbuf.c
    src/helpers/calculateGtk.c
    src/helpers/calculateTerminal.c
    src/helpers/calculateWM.c
    src/modules/break.c
    src/modules/custom.c
    src/modules/title.c
    src/modules/seperator.c
    src/modules/os.c
    src/modules/host.c
    src/modules/kernel.c
    src/modules/uptime.c
    src/modules/packages.c
    src/modules/shell.c
    src/modules/resolution.c
    src/modules/de.c
    src/modules/wm.c
    src/modules/wmtheme.c
    src/modules/theme.c
    src/modules/icons.c
    src/modules/font.c
    src/modules/terminal.c
    src/modules/terminalfont.c
    src/modules/cpu.c
    src/modules/gpu.c
    src/modules/memory.c
    src/modules/disk.c
    src/modules/battery.c
    src/modules/locale.c
    src/modules/colors.c
)

add_executable(fastfetch
    src/fastfetch.c
    src/util/FFvaluestore.c
    ${SRCS}
)

target_link_libraries(fastfetch
    ${CMAKE_DL_LIBS}
)

add_executable(flashfetch
    src/flashfetch.c
    ${SRCS}
)

target_compile_definitions(flashfetch PUBLIC
    FASTFETCH_BUILD_FLASHFETCH
)

target_link_libraries(flashfetch
    ${CMAKE_DL_LIBS}
)

if(BUILD_TESTS)
    add_executable(fastfetch-test-performance
        tests/performance.c
        ${SRCS}
    )

    target_link_libraries(fastfetch-test-performance
        ${CMAKE_DL_LIBS}
    )
endif(BUILD_TESTS)

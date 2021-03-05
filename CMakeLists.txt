cmake_minimum_required(VERSION 3.0)

project(fastfetch)

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

# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -march=native -O3 -pipe -fno-plt") # for testing

configure_file(src/fastfetch_config.h.in fastfetch_config.h)
include_directories(
    ${PROJECT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}/src
)

set(SRCS
    src/common.c
    src/logo.c
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
    src/modules/desktopenvironment.c
    src/modules/wm.c
    src/modules/theme.c
    src/modules/icons.c
    src/modules/font.c
    src/modules/terminal.c
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
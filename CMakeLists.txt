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
include_directories(${PROJECT_BINARY_DIR})

add_executable(${PROJECT_NAME}
    src/fastfetch.c
    src/break.c
    src/logo.c
    src/title.c
    src/seperator.c
    src/os.c
    src/host.c
    src/kernel.c
    src/uptime.c
    src/packages.c
    src/shell.c
    src/resolution.c
    src/desktopenvironment.c
    src/theme.c
    src/icons.c
    src/font.c
    src/terminal.c
    src/cpu.c
    src/gpu.c
    src/memory.c
    src/disk.c
    src/battery.c
    src/locale.c
    src/colors.c
)

target_link_libraries(${PROJECT_NAME}
    ${CMAKE_DL_LIBS}
    pci
)
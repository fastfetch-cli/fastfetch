cmake_minimum_required(VERSION 3.0)

project(fastfetch)

include (${CMAKE_ROOT}/Modules/FindX11.cmake)

# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -march=native -O3 -pipe -fno-plt") # for testing
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -march=native -O3 -pipe -fno-plt")

add_executable(fastfetch
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

target_link_libraries(fastfetch
    ${X11_LIBRARIES}
    pci
)
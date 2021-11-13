cmake_minimum_required(VERSION 3.1.0) # Threads::Threads

project(fastfetch LANGUAGES C)

option(BUILD_TESTS "Build test programs" OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-Wl,-O3")

if(BUILD_TESTS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native -mtune=native -pipe -fno-plt -Wconversion")
endif(BUILD_TESTS)

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
set(PROJECT_VERSION_MAJOR "${GIT_REV_LIST}")

function(fastfetch_load_text FILENAME OUTVAR)
    file(READ "${FILENAME}" TEMP)
    string(REPLACE "\n" "\\n" TEMP "${TEMP}")
    string(REPLACE "\"" "\\\"" TEMP "${TEMP}")
    string(REPLACE "$\\" "" TEMP "${TEMP}")
    set("${OUTVAR}" "${TEMP}" PARENT_SCOPE)
endfunction(fastfetch_load_text)

fastfetch_load_text(src/data/structure.txt DATATEXT_STRUCTURE)
fastfetch_load_text(src/data/config.txt DATATEXT_CONFIG)
fastfetch_load_text(src/data/modules.txt DATATEXT_MODULES)
fastfetch_load_text(src/data/help.txt DATATEXT_HELP)
fastfetch_load_text(src/data/help_color.txt DATATEXT_HELP_COLOR)
fastfetch_load_text(src/data/help_format.txt DATATEXT_HELP_FORMAT)
fastfetch_load_text(src/data/help_config.txt DATATEXT_HELP_CONFIG)

configure_file(src/fastfetch_config.h.in fastfetch_config.h)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_package(PkgConfig REQUIRED)

pkg_check_modules (LIBPCI libpci)
if(LIBPCI_FOUND)
    add_compile_definitions(FF_HAVE_LIBPCI=1)
else(LIBPCI_FOUND)
    message(WARNING "Package libpci not found. Building without support.")
endif(LIBPCI_FOUND)

pkg_check_modules (VULKAN vulkan)
if(VULKAN_FOUND)
    add_compile_definitions(FF_HAVE_VULKAN=1)
else(VULKAN_FOUND)
    message(WARNING "Package vulkan not found. Building without support.")
endif(VULKAN_FOUND)

pkg_check_modules (X11 x11)
if(X11_FOUND)
    add_compile_definitions(FF_HAVE_X11=1)
else(X11_FOUND)
    message(WARNING "Package x11 not found. Building without support.")
endif(X11_FOUND)

pkg_check_modules (XRANDR xrandr)
if(XRANDR_FOUND)
    add_compile_definitions(FF_HAVE_XRANDR=1)
else(XRANDR_FOUND)
    message(WARNING "Package xrandr not found. Building without support.")
endif(XRANDR_FOUND)

pkg_check_modules (WAYLAND wayland-client)
if(WAYLAND_FOUND)
    add_compile_definitions(FF_HAVE_WAYLAND=1)
else(WAYLAND_FOUND)
    message(WARNING "Package wayland-client not found. Building without support.")
endif(WAYLAND_FOUND)

pkg_check_modules (GIO gio-2.0)
if(GIO_FOUND)
    add_compile_definitions(FF_HAVE_GIO=1)
else(GIO_FOUND)
    message(WARNING "Package gio-2.0 not found. Building without support.")
endif(GIO_FOUND)

pkg_check_modules (DCONF dconf)
if(DCONF_FOUND)
    add_compile_definitions(FF_HAVE_DCONF=1)
else(DCONF_FOUND)
    message(WARNING "Package dconf not found. Building without support.")
endif(DCONF_FOUND)

pkg_check_modules (XFCONF libxfconf-0)
if(XFCONF_FOUND)
    add_compile_definitions(FF_HAVE_XFCONF=1)
else(XFCONF_FOUND)
    message(WARNING "Package libxfconf-0 not found. Building without support.")
endif(XFCONF_FOUND)

pkg_check_modules (RPM rpm)
if(RPM_FOUND)
    add_compile_definitions(FF_HAVE_RPM=1)
else(RPM_FOUND)
    message(WARNING "Package librpm not found. Building without support.")
endif(RPM_FOUND)

include_directories(
    ${PROJECT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}/src
    ${LIBPCI_INCLUDE_DIRS}
    ${X11_INCLUDE_DIRS}
    ${XRANDR_INCLUDE_DIRS}
    ${WAYLAND_INCLUDE_DIRS}
    ${GIO_INCLUDE_DIRS}
    ${DCONF_INCLUDE_DIRS}
    ${XFCONF_INCLUDE_DIRS}
    ${RPM_INCLUDE_DIRS}
)

link_libraries(
    ${CMAKE_DL_LIBS}
    Threads::Threads
)

set(SRCS
    src/util/FFstrbuf.c
    src/util/FFlist.c
    src/common/init.c
    src/common/threading.c
    src/common/io.c
    src/common/processing.c
    src/common/logo.c
    src/common/format.c
    src/common/parsing.c
    src/common/settings.c
    src/common/detectPlasma.c
    src/common/detectGTK.c
    src/common/detectWMDE.c
    src/common/detectTerminalShell.c
    src/modules/break.c
    src/modules/custom.c
    src/modules/title.c
    src/modules/separator.c
    src/modules/os.c
    src/modules/host.c
    src/modules/kernel.c
    src/modules/uptime.c
    src/modules/processes.c
    src/modules/packages.c
    src/modules/shell.c
    src/modules/resolution.c
    src/modules/de.c
    src/modules/wm.c
    src/modules/wmtheme.c
    src/modules/theme.c
    src/modules/icons.c
    src/modules/font.c
    src/modules/cursor.c
    src/modules/terminal.c
    src/modules/terminalfont.c
    src/modules/cpu.c
    src/modules/gpu.c
    src/modules/memory.c
    src/modules/disk.c
    src/modules/battery.c
    src/modules/locale.c
    src/modules/localip.c
    src/modules/colors.c
)

add_executable(fastfetch
    src/fastfetch.c
    src/util/FFvaluestore.c
    ${SRCS}
)

add_executable(flashfetch
    src/flashfetch.c
    ${SRCS}
)

target_compile_definitions(flashfetch PUBLIC
    FASTFETCH_BUILD_FLASHFETCH
)

if(BUILD_TESTS)
    add_executable(fastfetch-test-performance
        tests/performance.c
        ${SRCS}
    )
endif(BUILD_TESTS)

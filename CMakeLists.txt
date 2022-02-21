cmake_minimum_required(VERSION 3.1.0) # Threads::Threads

project(fastfetch LANGUAGES C)

include(GNUInstallDirs)

OPTION(ENABLE_LIBPCI "Enable libpci" ON)
OPTION(ENABLE_VULKAN "Enable vulkan" ON)
OPTION(ENABLE_WAYLAND "Enable wayland-client" ON)
OPTION(ENABLE_XCB_RANDR "Enable xcb-randr" ON)
OPTION(ENABLE_XCB "Enable xcb" ON)
OPTION(ENABLE_XRANDR "Enable xrandr" ON)
OPTION(ENABLE_X11 "Enable x11" ON)
OPTION(ENABLE_GIO "Enable gio-2.0" ON)
OPTION(ENABLE_DCONF "Enable dconf" ON)
OPTION(ENABLE_DBUS "Enable dbus-1" ON)
OPTION(ENABLE_XFCONF "Enable libxfconf-0" ON)
OPTION(ENABLE_RPM "Enable rpm" ON)
OPTION(BUILD_TESTS "Build tests" ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wconversion")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-Wl,-O3")

set(PROJECT_VERSION "r0.0" CACHE STRING "Full version")
set(PROJECT_VERSION_MAJOR "0" CACHE STRING "Major version")
if (EXISTS "${CMAKE_SOURCE_DIR}/.git")
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
endif()

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

# Init CMake targets.

add_library(libfastfetch STATIC
    src/util/FFstrbuf.c
    src/util/FFlist.c
    src/util/FFvaluestore.c
    src/common/init.c
    src/common/threading.c
    src/common/io.c
    src/common/processing.c
    src/common/logo.c
    src/common/format.c
    src/common/parsing.c
    src/common/settings.c
    src/common/library.c
    src/common/networking.c
    src/detection/os.c
    src/detection/plasma.c
    src/detection/gtk.c
    src/detection/terminalShell.c
    src/detection/media.c
    src/detection/displayserver/displayServer.c
    src/detection/displayserver/wayland.c
    src/detection/displayserver/xcb.c
    src/detection/displayserver/xlib.c
    src/detection/displayserver/wmde.c
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
    src/modules/cpuUsage.c
    src/modules/gpu.c
    src/modules/memory.c
    src/modules/disk.c
    src/modules/battery.c
    src/modules/locale.c
    src/modules/localip.c
    src/modules/publicip.c
    src/modules/player.c
    src/modules/song.c
    src/modules/colors.c
)

if(ENABLE_LIBPCI)
    pkg_check_modules (LIBPCI libpci)
    if(LIBPCI_FOUND)
        target_compile_definitions(libfastfetch PRIVATE FF_HAVE_LIBPCI=1)
    else(LIBPCI_FOUND)
        message(WARNING "Package libpci not found. Building without support.")
    endif(LIBPCI_FOUND)
endif(ENABLE_LIBPCI)

if(ENABLE_VULKAN)
    pkg_check_modules (VULKAN vulkan)
    if(VULKAN_FOUND)
        target_compile_definitions(libfastfetch PRIVATE FF_HAVE_VULKAN=1)
    else(VULKAN_FOUND)
        message(WARNING "Package vulkan not found. Building without support.")
    endif(VULKAN_FOUND)
endif(ENABLE_VULKAN)

if(ENABLE_WAYLAND)
    pkg_check_modules (WAYLAND wayland-client)
    if(WAYLAND_FOUND)
        target_compile_definitions(libfastfetch PRIVATE FF_HAVE_WAYLAND=1)
    else(WAYLAND_FOUND)
        message(WARNING "Package wayland-client not found. Building without support.")
    endif(WAYLAND_FOUND)
endif(ENABLE_WAYLAND)

if(ENABLE_XCB_RANDR)
    pkg_check_modules (XCB_RANDR xcb-randr)
    if(XCB_RANDR_FOUND)
        target_compile_definitions(libfastfetch PRIVATE FF_HAVE_XCB_RANDR=1)
    else(XCB_RANDR_FOUND)
        message(WARNING "Package xcb-randr not found. Building without support.")
    endif(XCB_RANDR_FOUND)
endif(ENABLE_XCB_RANDR)

if(ENABLE_XCB)
    pkg_check_modules (XCB xcb)
    if(XCB_FOUND)
        target_compile_definitions(libfastfetch PRIVATE FF_HAVE_XCB=1)
    else(XCB_FOUND)
        message(WARNING "Package xcb not found. Building without support.")
    endif(XCB_FOUND)
endif(ENABLE_XCB)

if(ENABLE_XRANDR)
    pkg_check_modules (XRANDR xrandr)
    if(XRANDR_FOUND)
        target_compile_definitions(libfastfetch PRIVATE FF_HAVE_XRANDR=1)
    else(XRANDR_FOUND)
        message(WARNING "Package xrandr not found. Building without support.")
    endif(XRANDR_FOUND)
endif(ENABLE_XRANDR)

if(ENABLE_X11)
    pkg_check_modules (X11 x11)
    if(X11_FOUND)
        target_compile_definitions(libfastfetch PRIVATE FF_HAVE_X11=1)
    else(X11_FOUND)
        message(WARNING "Package x11 not found. Building without support.")
    endif(X11_FOUND)
endif(ENABLE_X11)

if(ENABLE_GIO)
    pkg_check_modules (GIO gio-2.0)
    if(GIO_FOUND)
        target_compile_definitions(libfastfetch PRIVATE FF_HAVE_GIO=1)
    else(GIO_FOUND)
        message(WARNING "Package gio-2.0 not found. Building without support.")
    endif(GIO_FOUND)
endif(ENABLE_GIO)

if(ENABLE_DCONF)
    pkg_check_modules (DCONF dconf)
    if(DCONF_FOUND)
        target_compile_definitions(libfastfetch PRIVATE FF_HAVE_DCONF=1)
    else(DCONF_FOUND)
        message(WARNING "Package dconf not found. Building without support.")
    endif(DCONF_FOUND)
endif(ENABLE_DCONF)

if(ENABLE_DBUS)
    pkg_check_modules (DBUS dbus-1)
    if(DBUS_FOUND)
        target_compile_definitions(libfastfetch PRIVATE FF_HAVE_DBUS=1)
    else(DBUS_FOUND)
        message(WARNING "Package dbus-1 not found. Building without support.")
    endif(DBUS_FOUND)
endif(ENABLE_DBUS)

if(ENABLE_XFCONF)
    pkg_check_modules (XFCONF libxfconf-0)
    if(XFCONF_FOUND)
        target_compile_definitions(libfastfetch PRIVATE FF_HAVE_XFCONF=1)
    else(XFCONF_FOUND)
        message(WARNING "Package libxfconf-0 not found. Building without support.")
    endif(XFCONF_FOUND)
endif(ENABLE_XFCONF)

if(ENABLE_RPM)
    pkg_check_modules (RPM rpm)
    if(RPM_FOUND)
        target_compile_definitions(libfastfetch PRIVATE FF_HAVE_RPM=1)
    else(RPM_FOUND)
        message(WARNING "Package librpm not found. Building without support.")
    endif(RPM_FOUND)
endif(ENABLE_RPM)

target_include_directories(libfastfetch
    PUBLIC ${PROJECT_BINARY_DIR}
    PUBLIC ${PROJECT_SOURCE_DIR}/src

    PRIVATE ${LIBPCI_INCLUDE_DIRS}
    PRIVATE ${VULKAN_INCLUDE_DIRS}
    PRIVATE ${WAYLAND_INCLUDE_DIRS}
    PRIVATE ${XCB_RANDR_INCLUDE_DIRS}
    PRIVATE ${XCB_INCLUDE_DIRS}
    PRIVATE ${XRANDR_INCLUDE_DIRS}
    PRIVATE ${X11_INCLUDE_DIRS}
    PRIVATE ${GIO_INCLUDE_DIRS}
    PRIVATE ${DCONF_INCLUDE_DIRS}
    PRIVATE ${DBUS_INCLUDE_DIRS}
    PRIVATE ${XFCONF_INCLUDE_DIRS}
    PRIVATE ${RPM_INCLUDE_DIRS}
)

target_link_libraries(libfastfetch
    PRIVATE ${CMAKE_DL_LIBS}
    PRIVATE Threads::Threads
)

set_target_properties(libfastfetch PROPERTIES OUTPUT_NAME "fastfetch")

add_executable(fastfetch
    src/fastfetch.c
)
target_link_libraries(fastfetch
    PRIVATE libfastfetch
)

add_executable(flashfetch
    src/flashfetch.c
)
target_link_libraries(flashfetch
    PRIVATE libfastfetch
)

# Testing.

if (BUILD_TESTS)
    add_executable(fastfetch-test-performance
        tests/performance.c
    )
    target_link_libraries(fastfetch-test-performance
        PRIVATE libfastfetch
    )

    add_executable(fastfetch-test-strbuf
        tests/strbuf.c
    )
    target_link_libraries(fastfetch-test-strbuf
        PRIVATE libfastfetch
    )

    enable_testing()
    add_test(NAME test-strbuf COMMAND fastfetch-test-strbuf)
endif()

# Installation.

install(TARGETS fastfetch DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES completions/bash
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/bash-completion/completions
    RENAME ${CMAKE_PROJECT_NAME}
)

install(DIRECTORY presets
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${CMAKE_PROJECT_NAME}
)

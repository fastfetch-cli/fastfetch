cmake_minimum_required(VERSION 3.1.0) # Threads::Threads

project(fastfetch
    VERSION 1.3.4
    LANGUAGES C
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_package(PkgConfig REQUIRED)

# This is set to off by github actions for release builds
OPTION(SET_TWEAK "Add tweak to project version" ON)

# Also create test executables
OPTION(BUILD_TESTS "Build tests" OFF)

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
OPTION(ENABLE_SQLITE3 "Enable sqlite3" ON)
OPTION(ENABLE_IMAGEMAGICK7 "Enable imagemagick 7" ON)
OPTION(ENABLE_IMAGEMAGICK6 "Enable imagemagick 6" ON)
OPTION(ENABLE_ZLIB "Enable zlib" ON)
OPTION(ENABLE_CHAFA "Enable chafa" ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wconversion")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-Wl,-O3")

# These variables are mainly here for android, which has a different root fs structure.
if(NOT TARGET_DIR_ROOT)
    if(NOT ANDROID)
        set(TARGET_DIR_ROOT "")
    else()
        set(TARGET_DIR_ROOT "/data/data/com.termux/files/usr")
    endif()
endif()

if(NOT TARGET_DIR_USR)
    if(NOT ANDROID)
        set(TARGET_DIR_USR "${TARGET_DIR_ROOT}/usr")
    else()
        set(TARGET_DIR_USR "${TARGET_DIR_ROOT}")
    endif()
endif()

if(NOT TARGET_DIR_HOME)
    if(NOT ANDROID)
        set(TARGET_DIR_HOME "${TARGET_DIR_ROOT}/home")
    else()
        set(TARGET_DIR_HOME "/data/data/com.termux/files/home")
    endif()
endif()

function(fastfetch_load_text FILENAME OUTVAR)
    file(READ "${FILENAME}" TEMP)
    string(REPLACE "\n" "\\n" TEMP "${TEMP}")
    string(REPLACE "\"" "\\\"" TEMP "${TEMP}")
    string(REPLACE "$\\" "" TEMP "${TEMP}")
    set("${OUTVAR}" "${TEMP}" PARENT_SCOPE)
endfunction(fastfetch_load_text)

# Track commits between version bumps for output in --version
if (SET_TWEAK AND EXISTS "${CMAKE_SOURCE_DIR}/.git")
    execute_process(
        COMMAND git describe --tags
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        OUTPUT_VARIABLE PROJECT_VERSION_TWEAK
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    string(REGEX MATCH "-[0-9]+" PROJECT_VERSION_TWEAK "${PROJECT_VERSION_TWEAK}")
endif()

fastfetch_load_text(src/data/structure.txt DATATEXT_STRUCTURE)
fastfetch_load_text(src/data/config.txt DATATEXT_CONFIG)
fastfetch_load_text(src/data/modules.txt DATATEXT_MODULES)
fastfetch_load_text(src/data/help.txt DATATEXT_HELP)
fastfetch_load_text(src/data/help_color.txt DATATEXT_HELP_COLOR)
fastfetch_load_text(src/data/help_format.txt DATATEXT_HELP_FORMAT)
fastfetch_load_text(src/data/help_config.txt DATATEXT_HELP_CONFIG)

configure_file(src/fastfetch_config.h.in fastfetch_config.h)

# Init CMake targets

add_library(libfastfetch STATIC
    src/util/FFstrbuf.c
    src/util/FFlist.c
    src/util/FFvaluestore.c
    src/common/init.c
    src/common/threading.c
    src/common/io.c
    src/common/printing.c
    src/common/caching.c
    src/common/properties.c
    src/common/font.c
    src/common/processing.c
    src/common/format.c
    src/common/parsing.c
    src/common/settings.c
    src/common/library.c
    src/common/networking.c
    src/logo/logo.c
    src/logo/builtin.c
    src/logo/image/image.c
    src/logo/image/im7.c
    src/logo/image/im6.c
    src/detection/os.c
    src/detection/plasma.c
    src/detection/gtk.c
    src/detection/terminalShell.c
    src/detection/media.c
    src/detection/datetime.c
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
    src/modules/datetime.c
    src/modules/date.c
    src/modules/time.c
    src/modules/colors.c
)

if(ENABLE_LIBPCI)
    pkg_check_modules(LIBPCI libpci)
    if(LIBPCI_FOUND)
        target_compile_definitions(libfastfetch PRIVATE FF_HAVE_LIBPCI=1)
    else()
        message(WARNING "Package libpci not found. Building without support.")
    endif()
endif()

if(ENABLE_VULKAN)
    pkg_check_modules(VULKAN vulkan)
    if(VULKAN_FOUND)
        target_compile_definitions(libfastfetch PRIVATE FF_HAVE_VULKAN=1)
    else()
        message(WARNING "Package vulkan not found. Building without support.")
    endif()
endif()

if(ENABLE_WAYLAND)
    pkg_check_modules (WAYLAND wayland-client)
    if(WAYLAND_FOUND)
        target_compile_definitions(libfastfetch PRIVATE FF_HAVE_WAYLAND=1)
    else()
        message(WARNING "Package wayland-client not found. Building without support.")
    endif()
endif()

if(ENABLE_XCB_RANDR)
    pkg_check_modules(XCB_RANDR xcb-randr)
    if(XCB_RANDR_FOUND)
        target_compile_definitions(libfastfetch PRIVATE FF_HAVE_XCB_RANDR=1)
    else()
        message(WARNING "Package xcb-randr not found. Building without support.")
    endif()
endif()

if(ENABLE_XCB)
    pkg_check_modules (XCB xcb)
    if(XCB_FOUND)
        target_compile_definitions(libfastfetch PRIVATE FF_HAVE_XCB=1)
    else()
        message(WARNING "Package xcb not found. Building without support.")
    endif()
endif()

if(ENABLE_XRANDR)
    pkg_check_modules(XRANDR xrandr)
    if(XRANDR_FOUND)
        target_compile_definitions(libfastfetch PRIVATE FF_HAVE_XRANDR=1)
    else()
        message(WARNING "Package xrandr not found. Building without support.")
    endif()
endif()

if(ENABLE_X11)
    pkg_check_modules(X11 x11)
    if(X11_FOUND)
        target_compile_definitions(libfastfetch PRIVATE FF_HAVE_X11=1)
    else()
        message(WARNING "Package x11 not found. Building without support.")
    endif()
endif()

if(ENABLE_GIO)
    pkg_check_modules(GIO gio-2.0)
    if(GIO_FOUND)
        target_compile_definitions(libfastfetch PRIVATE FF_HAVE_GIO=1)
    else()
        message(WARNING "Package gio-2.0 not found. Building without support.")
    endif()
endif()

if(ENABLE_DCONF)
    pkg_check_modules(DCONF dconf)
    if(DCONF_FOUND)
        target_compile_definitions(libfastfetch PRIVATE FF_HAVE_DCONF=1)
    else()
        message(WARNING "Package dconf not found. Building without support.")
    endif()
endif()

if(ENABLE_DBUS)
    pkg_check_modules(DBUS dbus-1)
    if(DBUS_FOUND)
        target_compile_definitions(libfastfetch PRIVATE FF_HAVE_DBUS=1)
    else()
        message(WARNING "Package dbus-1 not found. Building without support.")
    endif()
endif()

if(ENABLE_XFCONF)
    pkg_check_modules(XFCONF libxfconf-0)
    if(XFCONF_FOUND)
        target_compile_definitions(libfastfetch PRIVATE FF_HAVE_XFCONF=1)
    else()
        message(WARNING "Package libxfconf-0 not found. Building without support.")
    endif()
endif()

if(ENABLE_SQLITE3)
    pkg_check_modules(SQLITE3 sqlite3)
    if(SQLITE3_FOUND)
        target_compile_definitions(libfastfetch PRIVATE FF_HAVE_SQLITE3=1)
    else()
        message(WARNING "Package sqlite3 not found. Building without support.")
    endif()
endif()

if(ENABLE_IMAGEMAGICK7)
    pkg_check_modules(IMAGEMAGICK7 MagickCore-7.Q16HDRI)
    if(NOT IMAGEMAGICK7_FOUND)
        pkg_check_modules(IMAGEMAGICK7 /usr/lib/imagemagick7/pkgconfig/MagickCore-7.Q16HDRI.pc)
    endif()
    if(NOT IMAGEMAGICK7_FOUND)
        pkg_check_modules(IMAGEMAGICK7 MagickCore-7.Q16)
    endif()
    if(NOT IMAGEMAGICK7_FOUND)
        pkg_check_modules(IMAGEMAGICK7 /usr/lib/imagemagick7/pkgconfig/MagickCore-7.Q16.pc)
    endif()

    if(IMAGEMAGICK7_FOUND)
        target_compile_definitions(libfastfetch PRIVATE FF_HAVE_IMAGEMAGICK7=1)
    else()
        message(WARNING "Package MagickCore-7.Q16HDRI or MagickCore-7.Q16 not found. Building without support.")
    endif()
endif()

if(ENABLE_IMAGEMAGICK6)
    pkg_check_modules(IMAGEMAGICK6 MagickCore-6.Q16HDRI)
    if(NOT IMAGEMAGICK6_FOUND)
        pkg_check_modules(IMAGEMAGICK6 /usr/lib/imagemagick6/pkgconfig/MagickCore-6.Q16HDRI.pc)
    endif()
    if(NOT IMAGEMAGICK6_FOUND)
        pkg_check_modules(IMAGEMAGICK6 MagickCore-6.Q16)
    endif()
    if(NOT IMAGEMAGICK6_FOUND)
        pkg_check_modules(IMAGEMAGICK6 /usr/lib/imagemagick6/pkgconfig/MagickCore-6.Q16.pc)
    endif()

    if(IMAGEMAGICK6_FOUND)
        target_compile_definitions(libfastfetch PRIVATE FF_HAVE_IMAGEMAGICK6=1)
    else()
        message(WARNING "Package MagickCore-6.Q16HDRI or MagickCore-6.Q16 not found. Building without support.")
    endif()
endif()

if(ENABLE_ZLIB)
    pkg_check_modules(ZLIB zlib)
    if(ZLIB_FOUND)
        target_compile_definitions(libfastfetch PRIVATE FF_HAVE_ZLIB=1)
    else()
        message(WARNING "Package zlib not found. Building without support")
    endif()
endif()

if(ENABLE_CHAFA)
    pkg_check_modules(CHAFA chafa>=1.10)
    if(CHAFA_FOUND)
        target_compile_definitions(libfastfetch PRIVATE FF_HAVE_CHAFA=1)
    else()
        message(WARNING "Package chafa>=1.10 not found. Building without support.")
    endif()
endif()

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
    PRIVATE ${SQLITE3_INCLUDE_DIRS}
    PRIVATE ${IMAGEMAGICK7_INCLUDE_DIRS}
    PRIVATE ${IMAGEMAGICK6_INCLUDE_DIRS}
    PRIVATE ${ZLIB_INCLUDE_DIRS}
    PRIVATE ${CHAFA_INCLUDE_DIRS}
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

# Testing

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

# Installation

include(GNUInstallDirs)

install(
    TARGETS fastfetch
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    TARGETS flashfetch
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    FILES completions/bash
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/bash-completion/completions
    RENAME ${CMAKE_PROJECT_NAME}
)

install(
    DIRECTORY presets
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${CMAKE_PROJECT_NAME}
)

install(
    FILES ./LICENSE
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/licenses/${CMAKE_PROJECT_NAME}
)

# Packaging

set(CPACK_GENERATOR "DEB;RPM;TGZ;ZIP")

set(CPACK_PACKAGE_CONTACT "Linus Dierheimer <Linus@Dierheimer.de>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Neofetch clone written in C")
set(CPACK_PACKAGE_DESCRIPTION "\
fastfetch is a neofetch-like tool for fetching system information and displaying them in a pretty way. \
It is written in c to achieve much better performance.\
")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/LinusDierheimer")

set(CPACK_DEBIAN_PACKAGE_SECTION, "utils")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6")
set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "\
libpci3, \
libvulkan1, \
libwayland-client0, \
libxcb-randr0, \
xcb, \
libxrandr2, \
libx11-6, \
libdconf1, \
libglib2.0-0, \
libdbus-1-3, \
libxfconf-0-3, \
libmagickcore-6.q16hdri-6, \
zlib1g \
")

set(CPACK_RPM_PACKAGE_LICENSE "MIT")

include(CPack)

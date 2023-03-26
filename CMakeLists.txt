cmake_minimum_required(VERSION 3.12.0) # target_link_libraries with OBJECT libs & project homepage url

project(fastfetch
    VERSION 1.11.3
    LANGUAGES C
    DESCRIPTION "Fast system information tool"
    HOMEPAGE_URL "https://github.com/LinusDierheimer/fastfetch"
)

set(PROJECT_LICENSE "MIT license")

###################
# Target Platform #
###################
if(ANDROID)
    set(LINUX FALSE)
elseif("${CMAKE_SYSTEM_NAME}" MATCHES ".*[Ll]inux.*")
    set(LINUX TRUE CACHE BOOL "..." FORCE) # LINUX means GNU/Linux, not just the kernel
elseif("${CMAKE_SYSTEM_NAME}" MATCHES ".*[Bb][Ss][Dd].*")
    set(BSD TRUE CACHE BOOL "..." FORCE)
elseif(NOT APPLE AND NOT WIN32)
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

if(WIN32)
    enable_language(CXX)
endif()

#############################
# Compile time dependencies #
#############################

set(THREADS_PREFER_PTHREAD_FLAG NOT WIN32)
find_package(Threads)

find_package(PkgConfig)
if(NOT PKG_CONFIG_FOUND)
    message(WARNING "pkg-config not found, library detection might be limited")
endif()

include(CheckIncludeFile)

#####################
# Configure options #
#####################

include(CMakeDependentOption)

cmake_dependent_option(ENABLE_LIBPCI "Enable libpci" ON "LINUX OR BSD" OFF)
cmake_dependent_option(ENABLE_VULKAN "Enable vulkan" ON "LINUX OR APPLE OR BSD OR WIN32 OR ANDROID" OFF)
cmake_dependent_option(ENABLE_WAYLAND "Enable wayland-client" ON "LINUX OR BSD" OFF)
cmake_dependent_option(ENABLE_XCB_RANDR "Enable xcb-randr" ON "LINUX OR BSD" OFF)
cmake_dependent_option(ENABLE_XCB "Enable xcb" ON "LINUX OR BSD" OFF)
cmake_dependent_option(ENABLE_XRANDR "Enable xrandr" ON "LINUX OR BSD" OFF)
cmake_dependent_option(ENABLE_X11 "Enable x11" ON "LINUX OR BSD" OFF)
cmake_dependent_option(ENABLE_GIO "Enable gio-2.0" ON "LINUX OR BSD" OFF)
cmake_dependent_option(ENABLE_DCONF "Enable dconf" ON "LINUX OR BSD" OFF)
cmake_dependent_option(ENABLE_DBUS "Enable dbus-1" ON "LINUX OR BSD" OFF)
cmake_dependent_option(ENABLE_XFCONF "Enable libxfconf-0" ON "LINUX OR BSD" OFF)
cmake_dependent_option(ENABLE_SQLITE3 "Enable sqlite3" ON "LINUX OR BSD OR APPLE" OFF)
cmake_dependent_option(ENABLE_RPM "Enable rpm" ON "LINUX" OFF)
cmake_dependent_option(ENABLE_IMAGEMAGICK7 "Enable imagemagick 7" ON "LINUX OR BSD OR APPLE OR WIN32" OFF)
cmake_dependent_option(ENABLE_IMAGEMAGICK6 "Enable imagemagick 6" ON "LINUX OR BSD OR APPLE" OFF)
cmake_dependent_option(ENABLE_CHAFA "Enable chafa" ON "ENABLE_IMAGEMAGICK6 OR ENABLE_IMAGEMAGICK7" OFF)
cmake_dependent_option(ENABLE_ZLIB "Enable zlib" ON "ENABLE_IMAGEMAGICK6 OR ENABLE_IMAGEMAGICK7" OFF)
cmake_dependent_option(ENABLE_EGL "Enable egl" ON "LINUX OR BSD" OFF)
cmake_dependent_option(ENABLE_GLX "Enable glx" ON "LINUX OR BSD" OFF)
cmake_dependent_option(ENABLE_OSMESA "Enable osmesa" ON "LINUX OR BSD" OFF)
cmake_dependent_option(ENABLE_OPENCL "Enable opencl" ON "LINUX OR BSD OR WIN32" OFF)
cmake_dependent_option(ENABLE_LIBNM "Enable libnm" ON "LINUX" OFF)
cmake_dependent_option(ENABLE_FREETYPE "Enable freetype" ON "ANDROID" OFF)
cmake_dependent_option(ENABLE_PULSE "Enable pulse" ON "LINUX OR BSD" OFF)
cmake_dependent_option(ENABLE_JSONC "Enable json-c" ON "LINUX OR APPLE OR BSD OR WIN32 OR ANDROID" OFF)
cmake_dependent_option(ENABLE_THREADS "Enable multithreading" ON "Threads_FOUND" OFF)
cmake_dependent_option(ENABLE_BUFFER "Enable stdout buffer" ON "LINUX OR APPLE OR BSD OR WIN32 OR ANDROID" OFF)

option(BUILD_TESTS "Build tests" OFF) # Also create test executables
option(SET_TWEAK "Add tweak to project version" ON) # This is set to off by github actions for release builds

####################
# Compiler options #
####################

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
if(ENABLE_THREADS)
    if(CMAKE_USE_WIN32_THREADS_INIT)
        message(STATUS "Threads type: Win32 thread")
    elseif(CMAKE_USE_PTHREADS_INIT)
        message(STATUS "Threads type: pthread")
    endif()
else()
    message(STATUS "Threads type: disabled")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wconversion")

if(WIN32)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wconversion -fno-exceptions -fno-rtti")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--tsaware -Wl,--build-id -Wl,--subsystem,console:6.1,--major-os-version,6,--minor-os-version,1")
endif()

# Used for dlopen finding dylibs installed by homebrew
# `/opt/homebrew/lib` is not on in dlopen search path by default
if(APPLE AND DEFINED ENV{HOMEBREW_PREFIX})
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,$ENV{HOMEBREW_PREFIX}/lib")
endif()

set(FASTFETCH_FLAGS_DEBUG "-fno-omit-frame-pointer")
if(NOT WIN32)
    set(FASTFETCH_FLAGS_DEBUG "${FASTFETCH_FLAGS_DEBUG} -fsanitize=address -fsanitize=undefined")
endif()
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${FASTFETCH_FLAGS_DEBUG}")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${FASTFETCH_FLAGS_DEBUG}")
if(NOT WIN32)
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -rdynamic")
endif()


if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT IPO_SUPPORTED)
    if(IPO_SUPPORTED)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endif()

#######################
# Target FS structure #
#######################

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
    if(APPLE)
        set(TARGET_DIR_HOME "${TARGET_DIR_ROOT}/Users")
    elseif(ANDROID)
        set(TARGET_DIR_HOME "/data/data/com.termux/files/home")
    else()
        set(TARGET_DIR_HOME "${TARGET_DIR_ROOT}/home")
    endif()
endif()

if(NOT TARGET_DIR_ETC)
    set(TARGET_DIR_ETC "${TARGET_DIR_ROOT}/etc")
endif()

message(STATUS "Target dirs: ROOT=\"${TARGET_DIR_ROOT}\" USR=\"${TARGET_DIR_USR}\" HOME=\"${TARGET_DIR_HOME}\" ETC=\"${TARGET_DIR_ETC}\"")

#https://cmake.org/cmake/help/latest/variable/CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT.html
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${TARGET_DIR_USR}" CACHE PATH "..." FORCE)
endif()

if(NOT CMAKE_INSTALL_SYSCONFDIR)
    set(CMAKE_INSTALL_SYSCONFDIR "${TARGET_DIR_ETC}" CACHE PATH "..." FORCE)
endif()

#################
# Tweak version #
#################

if (SET_TWEAK AND EXISTS "${CMAKE_SOURCE_DIR}/.git")
    execute_process(
        COMMAND git describe --tags
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        OUTPUT_VARIABLE PROJECT_VERSION_TWEAK
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    string(REGEX MATCH "-[0-9]+" PROJECT_VERSION_TWEAK "${PROJECT_VERSION_TWEAK}")
endif()
if(PROJECT_VERSION_TWEAK)
    string(REGEX MATCH "[0-9]+" PROJECT_VERSION_TWEAK_NUM "${PROJECT_VERSION_TWEAK}")
else()
    set(PROJECT_VERSION_TWEAK_NUM 0)
endif()

#############
# Text data #
#############

function(fastfetch_load_text FILENAME OUTVAR)
    file(READ "${FILENAME}" TEMP)
    string(REGEX REPLACE "\n$" "" TEMP "${TEMP}") # Remove trailing newline
    string(REPLACE "\n" "\\n" TEMP "${TEMP}")     # Replace newlines with \n
    string(REPLACE "\"" "\\\"" TEMP "${TEMP}")    # Replace quotes with \"
    string(REPLACE "$\\" "" TEMP "${TEMP}")       # Remove $\, so we can unescape some things
    set("${OUTVAR}" "${TEMP}" PARENT_SCOPE)
endfunction(fastfetch_load_text)

fastfetch_load_text(src/data/structure.txt DATATEXT_STRUCTURE)
fastfetch_load_text(src/data/config_system.txt DATATEXT_CONFIG_SYSTEM)
fastfetch_load_text(src/data/config_user.txt DATATEXT_CONFIG_USER)
fastfetch_load_text(src/data/modules.txt DATATEXT_MODULES)
fastfetch_load_text(src/data/help.txt DATATEXT_HELP)
fastfetch_load_text(src/data/help_color.txt DATATEXT_HELP_COLOR)
fastfetch_load_text(src/data/help_format.txt DATATEXT_HELP_FORMAT)
fastfetch_load_text(src/data/help_config.txt DATATEXT_HELP_CONFIG)

######################
# Configure config.h #
######################

configure_file(src/fastfetch_config.h.in fastfetch_config.h)

#######################
# libfastfetch target #
#######################

set(LIBFASTFETCH_SRC
    src/common/bar.c
    src/common/font.c
    src/common/format.c
    src/common/init.c
    src/common/json.c
    src/common/jsonconfig.c
    src/common/library.c
    src/common/option.c
    src/common/parsing.c
    src/common/printing.c
    src/common/properties.c
    src/common/settings.c
    src/detection/bluetooth/bluetooth.c
    src/detection/cpu/cpu.c
    src/detection/cpuusage/cpuusage.c
    src/detection/datetime/datetime.c
    src/detection/disk/disk.c
    src/detection/displayserver/displayserver.c
    src/detection/font/font.c
    src/detection/gpu/gpu.c
    src/detection/host/host.c
    src/detection/locale/locale.c
    src/detection/media/media.c
    src/detection/opencl/opencl.c
    src/detection/os/os.c
    src/detection/packages/packages.c
    src/detection/terminalfont/terminalfont.c
    src/detection/terminalshell/terminalshell.c
    src/detection/vulkan/vulkan.c
    src/logo/builtin.c
    src/logo/image/im6.c
    src/logo/image/im7.c
    src/logo/image/image.c
    src/logo/logo.c
    src/logo/option.c
    src/modules/battery/battery.c
    src/modules/bios/bios.c
    src/modules/bluetooth/bluetooth.c
    src/modules/board/board.c
    src/modules/brightness/brightness.c
    src/modules/break/break.c
    src/modules/chassis.c
    src/modules/colors/colors.c
    src/modules/cpu/cpu.c
    src/modules/cpuusage/cpuusage.c
    src/modules/cursor/cursor.c
    src/modules/custom/custom.c
    src/modules/command/command.c
    src/modules/datetime/datetime.c
    src/modules/de.c
    src/modules/disk.c
    src/modules/font.c
    src/modules/gpu/gpu.c
    src/modules/host/host.c
    src/modules/icons.c
    src/modules/gamepad.c
    src/modules/kernel/kernel.c
    src/modules/locale.c
    src/modules/localip.c
    src/modules/memory.c
    src/modules/opencl.c
    src/modules/opengl.c
    src/modules/os/os.c
    src/modules/packages.c
    src/modules/player.c
    src/modules/poweradapter.c
    src/modules/processes.c
    src/modules/publicip.c
    src/modules/display/display.c
    src/modules/separator/separator.c
    src/modules/shell.c
    src/modules/sound.c
    src/modules/swap.c
    src/modules/media.c
    src/modules/terminal.c
    src/modules/terminalfont.c
    src/modules/theme.c
    src/modules/title/title.c
    src/modules/uptime.c
    src/modules/users.c
    src/modules/vulkan.c
    src/modules/wallpaper.c
    src/modules/weather.c
    src/modules/wifi.c
    src/modules/wm.c
    src/modules/wmtheme.c
    src/modules/jsonconfig/jsonconfig.c
    src/util/FFlist.c
    src/util/FFstrbuf.c
    src/util/platform/FFPlatform.c
    src/util/stringUtils.c
)

if(LINUX)
    list(APPEND LIBFASTFETCH_SRC
        src/common/dbus.c
        src/common/io/io_unix.c
        src/common/networking_linux.c
        src/common/processing_linux.c
        src/detection/battery/battery_linux.c
        src/detection/bios/bios_linux.c
        src/detection/board/board_linux.c
        src/detection/brightness/brightness_linux.c
        src/detection/chassis/chassis_linux.c
        src/detection/cpu/cpu_linux.c
        src/detection/cpuusage/cpuusage_linux.c
        src/detection/cursor/cursor_linux.c
        src/detection/bluetooth/bluetooth_linux.c
        src/detection/disk/disk_linux.c
        src/detection/displayserver/linux/displayserver_linux.c
        src/detection/displayserver/linux/wayland.c
        src/detection/displayserver/linux/wmde.c
        src/detection/displayserver/linux/xcb.c
        src/detection/displayserver/linux/xlib.c
        src/detection/font/font_linux.c
        src/detection/gpu/gpu_linux.c
        src/detection/gtk_qt/gtk.c
        src/detection/host/host_linux.c
        src/detection/icons/icons_linux.c
        src/detection/localip/localip_linux.c
        src/detection/gamepad/gamepad_linux.c
        src/detection/media/media_linux.c
        src/detection/memory/memory_linux.c
        src/detection/opengl/opengl_linux.c
        src/detection/os/os_linux.c
        src/detection/packages/packages_linux.c
        src/detection/poweradapter/poweradapter_nosupport.c
        src/detection/processes/processes_linux.c
        src/detection/gtk_qt/qt.c
        src/detection/sound/sound_linux.c
        src/detection/swap/swap_linux.c
        src/detection/temps/temps_linux.c
        src/detection/terminalfont/terminalfont_linux.c
        src/detection/terminalshell/terminalshell_linux.c
        src/detection/theme/theme_linux.c
        src/detection/uptime/uptime_linux.c
        src/detection/users/users_linux.c
        src/detection/wallpaper/wallpaper_linux.c
        src/detection/wifi/wifi_linux.c
        src/detection/wmtheme/wmtheme_linux.c
        src/util/platform/FFPlatform_unix.c
    )
elseif(ANDROID)
    list(APPEND LIBFASTFETCH_SRC
        src/common/io/io_unix.c
        src/common/networking_linux.c
        src/common/processing_linux.c
        src/detection/battery/battery_android.c
        src/detection/bios/bios_nosupport.c
        src/detection/bluetooth/bluetooth_nosupport.c
        src/detection/board/board_nosupport.c
        src/detection/brightness/brightness_nosupport.c
        src/detection/chassis/chassis_nosupport.c
        src/detection/cpu/cpu_linux.c
        src/detection/cursor/cursor_nosupport.c
        src/detection/cpuusage/cpuusage_linux.c
        src/detection/disk/disk_linux.c
        src/detection/displayserver/displayserver_nosupport.c
        src/detection/font/font_nosupport.c
        src/detection/gpu/gpu_nosupport.c
        src/detection/host/host_android.c
        src/detection/icons/icons_nosupport.c
        src/detection/localip/localip_linux.c
        src/detection/gamepad/gamepad_nosupport.c
        src/detection/media/media_nosupport.c
        src/detection/memory/memory_linux.c
        src/detection/opengl/opengl_linux.c
        src/detection/os/os_android.c
        src/detection/packages/packages_linux.c
        src/detection/poweradapter/poweradapter_nosupport.c
        src/detection/processes/processes_linux.c
        src/detection/sound/sound_nosupport.c
        src/detection/swap/swap_linux.c
        src/detection/temps/temps_linux.c
        src/detection/terminalfont/terminalfont_android.c
        src/detection/terminalshell/terminalshell_linux.c
        src/detection/theme/theme_nosupport.c
        src/detection/uptime/uptime_linux.c
        src/detection/users/users_linux.c
        src/detection/wallpaper/wallpaper_nosupport.c
        src/detection/wifi/wifi_android.c
        src/detection/wmtheme/wmtheme_nosupport.c
        src/util/platform/FFPlatform_unix.c
    )
elseif(BSD)
    list(APPEND LIBFASTFETCH_SRC
        src/common/dbus.c
        src/common/io/io_unix.c
        src/common/networking_linux.c
        src/common/processing_linux.c
        src/common/sysctl.c
        src/detection/battery/battery_bsd.c
        src/detection/bios/bios_bsd.c
        src/detection/bluetooth/bluetooth_linux.c
        src/detection/board/board_nosupport.c
        src/detection/brightness/brightness_nosupport.c
        src/detection/chassis/chassis_nosupport.c
        src/detection/cpu/cpu_bsd.c
        src/detection/cpuusage/cpuusage_bsd.c
        src/detection/cursor/cursor_linux.c
        src/detection/disk/disk_bsd.c
        src/detection/displayserver/linux/displayserver_linux.c
        src/detection/displayserver/linux/wayland.c
        src/detection/displayserver/linux/wmde.c
        src/detection/displayserver/linux/xcb.c
        src/detection/displayserver/linux/xlib.c
        src/detection/font/font_linux.c
        src/detection/gpu/gpu_linux.c
        src/detection/gtk_qt/gtk.c
        src/detection/host/host_bsd.c
        src/detection/icons/icons_linux.c
        src/detection/localip/localip_linux.c
        src/detection/gamepad/gamepad_nosupport.c
        src/detection/media/media_linux.c
        src/detection/memory/memory_bsd.c
        src/detection/opengl/opengl_linux.c
        src/detection/os/os_linux.c
        src/detection/packages/packages_linux.c
        src/detection/poweradapter/poweradapter_nosupport.c
        src/detection/processes/processes_bsd.c
        src/detection/gtk_qt/qt.c
        src/detection/sound/sound_linux.c
        src/detection/swap/swap_bsd.c
        src/detection/temps/temps_linux.c
        src/detection/terminalfont/terminalfont_linux.c
        src/detection/terminalshell/terminalshell_linux.c
        src/detection/theme/theme_linux.c
        src/detection/uptime/uptime_bsd.c
        src/detection/users/users_linux.c
        src/detection/wallpaper/wallpaper_linux.c
        src/detection/wifi/wifi_nosupport.c
        src/detection/wmtheme/wmtheme_linux.c
        src/util/platform/FFPlatform_unix.c
    )
elseif(APPLE)
    list(APPEND LIBFASTFETCH_SRC
        src/common/io/io_unix.c
        src/common/networking_linux.c
        src/common/processing_linux.c
        src/common/sysctl.c
        src/detection/battery/battery_apple.c
        src/detection/bios/bios_apple.c
        src/detection/bluetooth/bluetooth_apple.m
        src/detection/board/board_nosupport.c
        src/detection/brightness/brightness_apple.c
        src/detection/chassis/chassis_nosupport.c
        src/detection/cpu/cpu_apple.c
        src/detection/cpuusage/cpuusage_apple.c
        src/detection/cursor/cursor_apple.m
        src/detection/disk/disk_apple.m
        src/detection/disk/disk_bsd.c
        src/detection/displayserver/displayserver_apple.c
        src/detection/font/font_apple.m
        src/detection/gpu/gpu_apple.c
        src/detection/host/host_apple.c
        src/detection/icons/icons_nosupport.c
        src/detection/localip/localip_linux.c
        src/detection/gamepad/gamepad_apple.c
        src/detection/media/media_apple.m
        src/detection/memory/memory_apple.c
        src/detection/opengl/opengl_apple.c
        src/detection/os/os_apple.m
        src/detection/packages/packages_apple.c
        src/detection/poweradapter/poweradapter_apple.c
        src/detection/processes/processes_bsd.c
        src/detection/sound/sound_apple.c
        src/detection/swap/swap_apple.c
        src/detection/temps/temps_apple.c
        src/detection/terminalfont/terminalfont_apple.m
        src/detection/terminalshell/terminalshell_linux.c
        src/detection/theme/theme_nosupport.c
        src/detection/uptime/uptime_bsd.c
        src/detection/users/users_linux.c
        src/detection/wallpaper/wallpaper_apple.c
        src/detection/wifi/wifi_apple.m
        src/detection/wmtheme/wmtheme_apple.m
        src/util/apple/cf_helpers.c
        src/util/apple/osascript.m
        src/util/platform/FFPlatform_unix.c
    )
elseif(WIN32)
    list(APPEND LIBFASTFETCH_SRC
        src/common/io/io_windows.c
        src/common/networking_windows.c
        src/common/processing_windows.c
        src/detection/battery/battery_windows.c
        src/detection/bios/bios_windows.c
        src/detection/bluetooth/bluetooth_windows.c
        src/detection/board/board_windows.c
        src/detection/brightness/brightness_windows.cpp
        src/detection/chassis/chassis_nosupport.c
        src/detection/cpu/cpu_windows.c
        src/detection/cpuusage/cpuusage_windows.c
        src/detection/cursor/cursor_windows.c
        src/detection/disk/disk_windows.c
        src/detection/displayserver/displayserver_windows.c
        src/detection/font/font_windows.c
        src/detection/gpu/gpu_windows.c
        src/detection/host/host_windows.c
        src/detection/icons/icons_windows.c
        src/detection/localip/localip_windows.c
        src/detection/gamepad/gamepad_windows.c
        src/detection/media/media_nosupport.c
        src/detection/memory/memory_windows.c
        src/detection/opengl/opengl_windows.c
        src/detection/os/os_windows.cpp
        src/detection/packages/packages_windows.c
        src/detection/poweradapter/poweradapter_nosupport.c
        src/detection/processes/processes_windows.c
        src/detection/sound/sound_windows.cpp
        src/detection/swap/swap_windows.c
        src/detection/terminalfont/terminalfont_windows.c
        src/detection/terminalshell/terminalshell_windows.c
        src/detection/temps/temps_windows.cpp
        src/detection/theme/theme_nosupport.c
        src/detection/uptime/uptime_windows.c
        src/detection/users/users_windows.c
        src/detection/wallpaper/wallpaper_windows.c
        src/detection/wifi/wifi_windows.c
        src/detection/wmtheme/wmtheme_windows.c
        src/util/windows/getline.c
        src/util/windows/com.cpp
        src/util/windows/registry.c
        src/util/windows/unicode.c
        src/util/windows/wmi.cpp
        src/util/platform/FFPlatform_windows.c
    )
endif()

add_library(libfastfetch OBJECT
    ${LIBFASTFETCH_SRC}
)

target_compile_definitions(libfastfetch PUBLIC _GNU_SOURCE)
if(WIN32)
    target_compile_definitions(libfastfetch PUBLIC WIN32_LEAN_AND_MEAN=1)
endif()

CHECK_INCLUDE_FILE("utmpx.h" HAVE_UTMPX_H)
if(HAVE_UTMPX_H)
    target_compile_definitions(libfastfetch PRIVATE FF_HAVE_UTMPX_H)
endif()

function(ff_lib_enable VARNAME PKGCONFIG_NAMES CMAKE_NAME)
    if(NOT ENABLE_${VARNAME})
        return()
    endif()

    if(PKG_CONFIG_FOUND)
        pkg_search_module(${VARNAME} QUIET ${PKGCONFIG_NAMES})
    endif()

    if(NOT ${VARNAME}_FOUND)
        find_package(${CMAKE_NAME} QUIET)

        set(${VARNAME}_FOUND ${${CMAKE_NAME}_FOUND})
        set(${VARNAME}_INCLUDE_DIRS ${${CMAKE_NAME}_INCLUDE_DIRS})
        set(${VARNAME}_LIBRARIES ${${CMAKE_NAME}_LIBRARIES})
        set(${VARNAME}_CFLAGS_OTHER ${${CMAKE_NAME}_CFLAGS_OTHER})
    endif()

    if(NOT ${VARNAME}_FOUND)
        message(STATUS "Library: missing: ${VARNAME}")
        return()
    endif()

    message(STATUS "Library: found ${VARNAME}")

    target_compile_definitions(libfastfetch PRIVATE FF_HAVE_${VARNAME}=1)
    target_include_directories(libfastfetch PRIVATE ${${VARNAME}_INCLUDE_DIRS})

    foreach(FLAG ${${VARNAME}_CFLAGS_OTHER})
        if(FLAG MATCHES "-D.*")
            string(SUBSTRING ${FLAG} 2 -1 FLAG)
            target_compile_definitions(libfastfetch PRIVATE ${FLAG})
        endif()
    endforeach()
endfunction()

ff_lib_enable(LIBPCI
    "libpci"
    "Pci"
)
ff_lib_enable(VULKAN
    "vulkan"
    "Vulkan"
)
ff_lib_enable(WAYLAND
    "wayland-client"
    "WaylandClient"
)
ff_lib_enable(XCB_RANDR
    "xcb-randr"
    "XcbRandr"
)
ff_lib_enable(XCB
    "xcb"
    "Xcb"
)
ff_lib_enable(XRANDR
    "xrandr"
    "XRandr"
)
ff_lib_enable(X11
    "x11"
    "X11"
)
ff_lib_enable(GIO
    "gio-2.0"
    "GIO"
)
ff_lib_enable(DCONF
    "dconf"
    "DConf"
)
ff_lib_enable(DBUS
    "dbus-1"
    "DBus"
)
ff_lib_enable(XFCONF
    "libxfconf-0"
    "XFConf"
)
ff_lib_enable(SQLITE3
    "sqlite3"
    "SQLite3"
)
ff_lib_enable(RPM
    "rpm"
    "RPM"
)
ff_lib_enable(IMAGEMAGICK7
    "MagickCore-7.Q16HDRI;MagickCore-7.Q16;MagickCore-7;/usr/lib/imagemagick7/pkgconfig/MagickCore-7.Q16HDRI.pc;/usr/lib/imagemagick7/pkgconfig/MagickCore-7.Q16.pc;/usr/lib/imagemagick7/pkgconfig/MagickCore-7.pc"
    "ImageMagick7"
)
ff_lib_enable(IMAGEMAGICK6
    "MagickCore-6.Q16HDRI;MagickCore-6.Q16;MagickCore-6;/usr/lib/imagemagick6/pkgconfig/MagickCore-6.Q16HDRI.pc;/usr/lib/imagemagick6/pkgconfig/MagickCore-6.Q16.pc;/usr/lib/imagemagick6/pkgconfig/MagickCore-6.pc"
    "ImageMagick6"
)
ff_lib_enable(ZLIB
    "zlib"
    "ZLIB"
)
ff_lib_enable(CHAFA
    "chafa>=1.10"
    "Chafa"
)
ff_lib_enable(EGL
    "egl"
    "EGL"
)
ff_lib_enable(GLX
    "glx"
    "GLX"
)
ff_lib_enable(OSMESA
    "osmesa"
    "OSMesa"
)
ff_lib_enable(OPENCL
    "OpenCL"
    "OpenCL"
)
ff_lib_enable(LIBNM
    "libnm"
    "libnm"
)
ff_lib_enable(FREETYPE
    "freetype2"
    "FreeType2"
)
ff_lib_enable(PULSE
    "libpulse"
    "Pulse"
)
ff_lib_enable(JSONC
    "json-c"
    "json-c"
)

if(ENABLE_THREADS)
    target_compile_definitions(libfastfetch PRIVATE FF_HAVE_THREADS)
    if(CMAKE_USE_PTHREADS_INIT) #Threads::Threads is not set for WIN32
        target_link_libraries(libfastfetch PRIVATE Threads::Threads)
    endif()
endif()

if(ENABLE_BUFFER)
    target_compile_definitions(libfastfetch PRIVATE FF_ENABLE_BUFFER)
endif()

if(APPLE)
    target_link_libraries(libfastfetch
        PRIVATE "-framework Cocoa"
        PRIVATE "-framework CoreFoundation"
        PRIVATE "-framework CoreAudio"
        PRIVATE "-framework CoreVideo"
        PRIVATE "-framework CoreWLAN"
        PRIVATE "-framework IOBluetooth"
        PRIVATE "-framework IOKit"
        PRIVATE "-framework OpenGL"
        PRIVATE "-framework OpenCL"
        PRIVATE "-framework SystemConfiguration"
        PRIVATE "-weak_framework CoreDisplay"
        PRIVATE "-weak_framework DisplayServices -F /System/Library/PrivateFrameworks"
        PRIVATE "-weak_framework MediaRemote -F /System/Library/PrivateFrameworks"
    )
elseif(WIN32)
    target_compile_definitions(libfastfetch PRIVATE -D_WIN32_WINNT=0x0601)
    target_link_libraries(libfastfetch
        PRIVATE "bthprops"
        PRIVATE "dwmapi"
        PRIVATE "gdi32"
        PRIVATE "iphlpapi"
        PRIVATE "ole32"
        PRIVATE "oleaut32"
        PRIVATE "opengl32"
        PRIVATE "ws2_32"
        PRIVATE "ntdll"
        PRIVATE "version"
        PRIVATE "setupapi"
        PRIVATE "hid"
        PRIVATE "wtsapi32"
        PRIVATE "powrprof"
    )
endif()

target_include_directories(libfastfetch
    PUBLIC ${PROJECT_BINARY_DIR}
    PUBLIC ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(libfastfetch
    PRIVATE ${CMAKE_DL_LIBS}
)

######################
# Executable targets #
######################

add_executable(fastfetch
    src/fastfetch.c
)
target_compile_definitions(fastfetch
    PRIVATE FASTFETCH_TARGET_BINARY_NAME=fastfetch
)
target_link_libraries(fastfetch
    PRIVATE libfastfetch
)

add_executable(flashfetch
    src/flashfetch.c
)
target_compile_definitions(flashfetch
    PRIVATE FASTFETCH_TARGET_BINARY_NAME=flashfetch
)
target_link_libraries(flashfetch
    PRIVATE libfastfetch
)

if(ENABLE_BUFFER) # FF_ENABLE_BUFFER is used in fastfetch.c
    target_compile_definitions(fastfetch PRIVATE FF_ENABLE_BUFFER)
    target_compile_definitions(flashfetch PRIVATE FF_ENABLE_BUFFER)
endif()

if(WIN32)
    set(TARGET_NAME fastfetch)
    target_sources(fastfetch
        PRIVATE src/util/windows/version.rc
    )

    set(TARGET_NAME flashfetch)
    target_sources(flashfetch
        PRIVATE src/util/windows/version.rc
    )
endif()

###################
# Testing targets #
###################

if (BUILD_TESTS)
    add_executable(fastfetch-test-strbuf
        tests/strbuf.c
    )
    target_link_libraries(fastfetch-test-strbuf
        PRIVATE libfastfetch
    )

    add_executable(fastfetch-test-list
        tests/list.c
    )
    target_link_libraries(fastfetch-test-list
        PRIVATE libfastfetch
    )

    enable_testing()
    add_test(NAME test-strbuf COMMAND fastfetch-test-strbuf)
    add_test(NAME test-list COMMAND fastfetch-test-list)
endif()

##################
# install target #
##################

include(GNUInstallDirs)

install(
    TARGETS fastfetch flashfetch
    DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

install(
    FILES "${CMAKE_SOURCE_DIR}/src/data/config_system.txt"
    DESTINATION "${CMAKE_INSTALL_SYSCONFDIR}/${CMAKE_PROJECT_NAME}"
    RENAME "config.conf"
)

install(
    FILES "${CMAKE_SOURCE_DIR}/completions/bash"
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/bash-completion/completions"
    RENAME "${CMAKE_PROJECT_NAME}"
)

install(
    DIRECTORY "${CMAKE_SOURCE_DIR}/presets"
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/${CMAKE_PROJECT_NAME}"
)

install(
    FILES "${CMAKE_SOURCE_DIR}/LICENSE"
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/licenses/${CMAKE_PROJECT_NAME}"
)

##################
# package target #
##################

set(CPACK_GENERATOR "TGZ;ZIP")

if(LINUX)
    set(CPACK_GENERATOR "${CPACK_GENERATOR};DEB;RPM")

    set(CPACK_DEBIAN_PACKAGE_SECTION, "utils")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6")

    set(CPACK_RPM_PACKAGE_LICENSE "MIT")
endif()

set(CPACK_SET_DESTDIR ON)

set(CPACK_PACKAGE_CONTACT "Linus Dierheimer <Linus@Dierheimer.de>")
set(CPACK_PACKAGE_DESCRIPTION "\
fastfetch is a neofetch-like tool for fetching system information and displaying them in a pretty way. \
It is written in c to achieve much better performance.\
")

include(CPack)

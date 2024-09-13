cmake_minimum_required(VERSION 3.12.0) # target_link_libraries with OBJECT libs & project homepage url

project(fastfetch
    VERSION 2.24.0
    LANGUAGES C
    DESCRIPTION "Fast neofetch-like system information tool"
    HOMEPAGE_URL "https://github.com/fastfetch-cli/fastfetch"
)

set(PROJECT_LICENSE "MIT license")

###################
# Target Platform #
###################
if(ANDROID)
    set(LINUX FALSE)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    set(LINUX TRUE CACHE BOOL "..." FORCE) # LINUX means GNU/Linux, not just the kernel
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "FreeBSD")
    set(FreeBSD TRUE CACHE BOOL "..." FORCE)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "SunOS")
    set(SunOS TRUE CACHE BOOL "..." FORCE)
elseif(NOT APPLE AND NOT WIN32)
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
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

cmake_dependent_option(ENABLE_VULKAN "Enable vulkan" ON "LINUX OR APPLE OR FreeBSD OR WIN32 OR ANDROID OR SunOS" OFF)
cmake_dependent_option(ENABLE_WAYLAND "Enable wayland-client" ON "LINUX OR FreeBSD" OFF)
cmake_dependent_option(ENABLE_XCB_RANDR "Enable xcb-randr" ON "LINUX OR FreeBSD OR SunOS" OFF)
cmake_dependent_option(ENABLE_XCB "Enable xcb" ON "LINUX OR FreeBSD OR SunOS" OFF)
cmake_dependent_option(ENABLE_XRANDR "Enable xrandr" ON "LINUX OR FreeBSD OR SunOS" OFF)
cmake_dependent_option(ENABLE_X11 "Enable x11" ON "LINUX OR FreeBSD OR SunOS" OFF)
cmake_dependent_option(ENABLE_DRM "Enable libdrm" ON "LINUX OR FreeBSD OR SunOS" OFF)
cmake_dependent_option(ENABLE_GIO "Enable gio-2.0" ON "LINUX OR FreeBSD OR SunOS" OFF)
cmake_dependent_option(ENABLE_DCONF "Enable dconf" ON "LINUX OR FreeBSD OR SunOS" OFF)
cmake_dependent_option(ENABLE_DBUS "Enable dbus-1" ON "LINUX OR FreeBSD OR SunOS" OFF)
cmake_dependent_option(ENABLE_XFCONF "Enable libxfconf-0" ON "LINUX OR FreeBSD OR SunOS" OFF)
cmake_dependent_option(ENABLE_SQLITE3 "Enable sqlite3" ON "LINUX OR FreeBSD OR APPLE OR SunOS" OFF)
cmake_dependent_option(ENABLE_RPM "Enable rpm" ON "LINUX" OFF)
cmake_dependent_option(ENABLE_IMAGEMAGICK7 "Enable imagemagick 7" ON "LINUX OR FreeBSD OR APPLE OR WIN32 OR SunOS" OFF)
cmake_dependent_option(ENABLE_IMAGEMAGICK6 "Enable imagemagick 6" ON "LINUX OR FreeBSD OR APPLE OR SunOS" OFF)
cmake_dependent_option(ENABLE_CHAFA "Enable chafa" ON "ENABLE_IMAGEMAGICK6 OR ENABLE_IMAGEMAGICK7" OFF)
cmake_dependent_option(ENABLE_ZLIB "Enable zlib" ON "ENABLE_IMAGEMAGICK6 OR ENABLE_IMAGEMAGICK7" OFF)
cmake_dependent_option(ENABLE_EGL "Enable egl" ON "LINUX OR FreeBSD OR WIN32 OR SunOS" OFF)
cmake_dependent_option(ENABLE_GLX "Enable glx" ON "LINUX OR FreeBSD OR SunOS" OFF)
cmake_dependent_option(ENABLE_OSMESA "Enable osmesa" ON "LINUX OR FreeBSD OR SunOS" OFF)
cmake_dependent_option(ENABLE_OPENCL "Enable opencl" ON "LINUX OR FreeBSD OR WIN32 OR ANDROID OR SunOS" OFF)
cmake_dependent_option(ENABLE_FREETYPE "Enable freetype" ON "ANDROID" OFF)
cmake_dependent_option(ENABLE_PULSE "Enable pulse" ON "LINUX OR SunOS" OFF)
cmake_dependent_option(ENABLE_DDCUTIL "Enable ddcutil" ON "LINUX" OFF)
cmake_dependent_option(ENABLE_DIRECTX_HEADERS "Enable DirectX headers for WSL" ON "LINUX" OFF)
cmake_dependent_option(ENABLE_ELF "Enable libelf" ON "LINUX OR ANDROID" OFF)
cmake_dependent_option(ENABLE_THREADS "Enable multithreading" ON "Threads_FOUND" OFF)
cmake_dependent_option(ENABLE_LIBZFS "Enable libzfs" ON "LINUX OR FreeBSD OR SunOS" OFF)

option(ENABLE_SYSTEM_YYJSON "Use system provided (instead of fastfetch embedded) yyjson library" OFF)
option(ENABLE_ASAN "Build fastfetch with ASAN (address sanitizer)" OFF)
option(ENABLE_LTO "Enable link-time optimization in release mode if supported" ON)
option(BUILD_TESTS "Build tests" OFF) # Also create test executables
option(SET_TWEAK "Add tweak to project version" ON) # This is set to off by github actions for release builds
option(IS_MUSL "Build with musl libc" OFF) # Used by Github Actions
option(INSTALL_LICENSE "Install license into /usr/share/licenses" ON)

set(BINARY_LINK_TYPE_OPTIONS dlopen dynamic static)
set(BINARY_LINK_TYPE dlopen CACHE STRING "How to link fastfetch")
set_property(CACHE BINARY_LINK_TYPE PROPERTY STRINGS ${BINARY_LINK_TYPE_OPTIONS})
if(NOT BINARY_LINK_TYPE IN_LIST BINARY_LINK_TYPE_OPTIONS)
    message(FATAL_ERROR "BINARY_LINK_TYPE must be one of ${BINARY_LINK_TYPE_OPTIONS}")
endif()

set(PACKAGE_MANAGERS AM APK BREW CHOCO DPKG EMERGE EOPKG FLATPAK GUIX LINGLONG LPKG LPKGBUILD MACPORTS NIX OPKG PACMAN PALUDIS PKG PKGTOOL RPM SCOOP SNAP SORCERY WINGET XBPS)
foreach(package_manager ${PACKAGE_MANAGERS})
    if(package_manager STREQUAL "WINGET")
        option(PACKAGES_DISABLE_${package_manager} "Disable ${package_manager} package manager detection by default" ON)
    else()
        option(PACKAGES_DISABLE_${package_manager} "Disable ${package_manager} package manager detection by default" OFF)
    endif()
endforeach()

if (LINUX)
    set(CUSTOM_PCI_IDS_PATH "" CACHE STRING "Custom path to file pci.ids, defaults to `/usr/share/hwdata/pci.ids`")
    set(CUSTOM_AMDGPU_IDS_PATH "" CACHE STRING "Custom path to file amdgpu.ids, defaults to `/usr/share/libdrm/amdgpu.ids`")
    set(CUSTOM_OS_RELEASE_PATH "" CACHE STRING "Custom path to file os-release, defaults to `/etc/os-release`")
endif()

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

set(WARNING_FLAGS "-Wall -Wextra -Wconversion -Werror=uninitialized -Werror=return-type")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARNING_FLAGS} -Werror=incompatible-pointer-types -Werror=implicit-function-declaration -Werror=int-conversion")

if(WIN32 OR ENABLE_DIRECTX_HEADERS)
    enable_language(CXX)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS}")
endif()

if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--tsaware -Wl,--build-id -Wl,--subsystem,console:6.1,--major-os-version,6,--minor-os-version,1")
elseif(APPLE AND CMAKE_C_COMPILER_ID MATCHES "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fobjc-arc")
endif()

set(FASTFETCH_FLAGS_DEBUG "-fno-omit-frame-pointer")
if(ENABLE_ASAN)
    message(STATUS "Address sanitizer enabled (DEBUG only)")
    set(FASTFETCH_FLAGS_DEBUG "${FASTFETCH_FLAGS_DEBUG} -fsanitize=address")
endif()
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${FASTFETCH_FLAGS_DEBUG}")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${FASTFETCH_FLAGS_DEBUG} -fstack-protector-all -fno-delete-null-pointer-checks")
if(NOT WIN32)
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -rdynamic")
endif()

if(ENABLE_LTO AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Enabling LTO")
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
        OUTPUT_VARIABLE PROJECT_VERSION_GIT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    string(REGEX MATCH "-[0-9]+" PROJECT_VERSION_TWEAK "${PROJECT_VERSION_GIT}")
endif()
if(PROJECT_VERSION_TWEAK)
    string(REGEX MATCH "[0-9]+" PROJECT_VERSION_TWEAK_NUM "${PROJECT_VERSION_TWEAK}")
else()
    set(PROJECT_VERSION_TWEAK_NUM 0)
endif()

#############
# Text data #
#############

function(fastfetch_encode_c_string STR OUTVAR)
    string(REGEX REPLACE "\n$" "" TEMP "${STR}")  # Remove trailing newline
    string(REPLACE "\\" "\\\\" TEMP "${TEMP}")    # Escape backslashes
    string(REPLACE "\n" "\\n" TEMP "${TEMP}")     # Replace newlines with \n
    string(REPLACE "\"" "\\\"" TEMP "${TEMP}")    # Replace quotes with \"
    set(${OUTVAR} "\"${TEMP}\"" PARENT_SCOPE)
endfunction(fastfetch_encode_c_string)

function(fastfetch_load_text FILENAME OUTVAR)
    file(READ "${FILENAME}" TEMP)
    fastfetch_encode_c_string("${TEMP}" TEMP)
    set(${OUTVAR} "${TEMP}" PARENT_SCOPE)
endfunction(fastfetch_load_text)

find_package(Python)
if(Python_FOUND)
    execute_process(COMMAND ${Python_EXECUTABLE} -c "import json,sys;json.dump(json.load(sys.stdin),sys.stdout,separators=(',',':'))"
                    INPUT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/src/data/help.json"
                    OUTPUT_VARIABLE DATATEXT_JSON_HELP)
    if(DATATEXT_JSON_HELP STREQUAL "")
        message(ERROR "DATATEXT_JSON_HELP is empty, which should not happen!")
    endif()
else()
    message(STATUS "Python3 is not found, 'help.json' will not be minified")
    file(READ "src/data/help.json" DATATEXT_JSON_HELP)
endif()

fastfetch_encode_c_string("${DATATEXT_JSON_HELP}" DATATEXT_JSON_HELP)
fastfetch_load_text(src/data/structure.txt DATATEXT_STRUCTURE)
fastfetch_load_text(src/data/help_footer.txt DATATEXT_HELP_FOOTER)
fastfetch_load_text(src/data/help_color.txt DATATEXT_HELP_COLOR)
fastfetch_load_text(src/data/help_format.txt DATATEXT_HELP_FORMAT)

configure_file(src/fastfetch_config.h.in fastfetch_config.h @ONLY)
configure_file(src/fastfetch_datatext.h.in fastfetch_datatext.h @ONLY)
if(APPLE)
    configure_file(src/util/apple/Info.plist.in Info.plist @ONLY)
endif()

string(TIMESTAMP FASTFETCH_BUILD_DATE "%d %B %Y" UTC)
configure_file(doc/fastfetch.1.in fastfetch.1 @ONLY)

####################
# Ascii image data #
####################

file(GLOB LOGO_FILES "src/logo/ascii/*.txt")
set(LOGO_BUILTIN_H "#pragma once\n#pragma GCC diagnostic ignored \"-Wtrigraphs\"\n\n")
foreach(file ${LOGO_FILES})
    fastfetch_load_text("${file}" content)
    get_filename_component(file "${file}" NAME_WE)
    string(TOUPPER "${file}" file)
    string(REGEX REPLACE "\\$\\{c([0-9]+)\\}" "$\\1" content "${content}")
    set(LOGO_BUILTIN_H "${LOGO_BUILTIN_H}#define FASTFETCH_DATATEXT_LOGO_${file} ${content}\n")
endforeach()
file(GENERATE OUTPUT logo_builtin.h CONTENT "${LOGO_BUILTIN_H}")

#######################
# libfastfetch target #
#######################

set(LIBFASTFETCH_SRC
    src/common/percent.c
    src/common/commandoption.c
    src/common/font.c
    src/common/format.c
    src/common/init.c
    src/common/jsonconfig.c
    src/common/library.c
    src/common/modules.c
    src/common/netif/netif.c
    src/common/option.c
    src/common/parsing.c
    src/common/printing.c
    src/common/properties.c
    src/common/settings.c
    src/common/temps.c
    src/detection/bluetoothradio/bluetoothradio.c
    src/detection/bootmgr/bootmgr.c
    src/detection/chassis/chassis.c
    src/detection/cpu/cpu.c
    src/detection/cpuusage/cpuusage.c
    src/detection/disk/disk.c
    src/detection/diskio/diskio.c
    src/detection/displayserver/displayserver.c
    src/detection/editor/editor.c
    src/detection/font/font.c
    src/detection/gpu/gpu.c
    src/detection/media/media.c
    src/detection/netio/netio.c
    src/detection/opencl/opencl.c
    src/detection/opengl/opengl_shared.c
    src/detection/os/os.c
    src/detection/packages/packages.c
    src/detection/physicalmemory/physicalmemory.c
    src/detection/publicip/publicip.c
    src/detection/terminaltheme/terminaltheme.c
    src/detection/terminalfont/terminalfont.c
    src/detection/terminalshell/terminalshell.c
    src/detection/version/version.c
    src/detection/vulkan/vulkan.c
    src/detection/weather/weather.c
    src/logo/builtin.c
    src/logo/image/im6.c
    src/logo/image/im7.c
    src/logo/image/image.c
    src/logo/logo.c
    src/modules/battery/battery.c
    src/modules/bios/bios.c
    src/modules/bluetooth/bluetooth.c
    src/modules/bluetoothradio/bluetoothradio.c
    src/modules/board/board.c
    src/modules/bootmgr/bootmgr.c
    src/modules/brightness/brightness.c
    src/modules/break/break.c
    src/modules/camera/camera.c
    src/modules/chassis/chassis.c
    src/modules/colors/colors.c
    src/modules/cpu/cpu.c
    src/modules/cpucache/cpucache.c
    src/modules/cpuusage/cpuusage.c
    src/modules/cursor/cursor.c
    src/modules/custom/custom.c
    src/modules/command/command.c
    src/modules/datetime/datetime.c
    src/modules/de/de.c
    src/modules/disk/disk.c
    src/modules/diskio/diskio.c
    src/modules/dns/dns.c
    src/modules/editor/editor.c
    src/modules/font/font.c
    src/modules/gpu/gpu.c
    src/modules/host/host.c
    src/modules/icons/icons.c
    src/modules/initsystem/initsystem.c
    src/modules/gamepad/gamepad.c
    src/modules/kernel/kernel.c
    src/modules/lm/lm.c
    src/modules/loadavg/loadavg.c
    src/modules/locale/locale.c
    src/modules/localip/localip.c
    src/modules/memory/memory.c
    src/modules/monitor/monitor.c
    src/modules/netio/netio.c
    src/modules/opencl/opencl.c
    src/modules/opengl/opengl.c
    src/modules/os/os.c
    src/modules/packages/packages.c
    src/modules/physicaldisk/physicaldisk.c
    src/modules/physicalmemory/physicalmemory.c
    src/modules/processes/processes.c
    src/modules/player/player.c
    src/modules/poweradapter/poweradapter.c
    src/modules/publicip/publicip.c
    src/modules/display/display.c
    src/modules/separator/separator.c
    src/modules/shell/shell.c
    src/modules/sound/sound.c
    src/modules/swap/swap.c
    src/modules/media/media.c
    src/modules/terminal/terminal.c
    src/modules/terminaltheme/terminaltheme.c
    src/modules/terminalfont/terminalfont.c
    src/modules/terminalsize/terminalsize.c
    src/modules/theme/theme.c
    src/modules/title/title.c
    src/modules/uptime/uptime.c
    src/modules/users/users.c
    src/modules/version/version.c
    src/modules/vulkan/vulkan.c
    src/modules/wallpaper/wallpaper.c
    src/modules/weather/weather.c
    src/modules/wifi/wifi.c
    src/modules/wm/wm.c
    src/modules/wmtheme/wmtheme.c
    src/modules/zpool/zpool.c
    src/options/display.c
    src/options/modules.c
    src/options/logo.c
    src/options/general.c
    src/util/edidHelper.c
    src/util/base64.c
    src/util/FFlist.c
    src/util/FFstrbuf.c
    src/util/path.c
    src/util/platform/FFPlatform.c
    src/util/smbiosHelper.c
)

if(LINUX)
    list(APPEND LIBFASTFETCH_SRC
        src/common/dbus.c
        src/common/io/io_unix.c
        src/common/netif/netif_linux.c
        src/common/networking_linux.c
        src/common/processing_linux.c
        src/detection/battery/battery_linux.c
        src/detection/bios/bios_linux.c
        src/detection/board/board_linux.c
        src/detection/bootmgr/bootmgr_linux.c
        src/detection/brightness/brightness_linux.c
        src/detection/chassis/chassis_linux.c
        src/detection/cpu/cpu_linux.c
        src/detection/cpucache/cpucache_linux.c
        src/detection/cpuusage/cpuusage_linux.c
        src/detection/cursor/cursor_linux.c
        src/detection/bluetooth/bluetooth_linux.c
        src/detection/bluetoothradio/bluetoothradio_linux.c
        src/detection/disk/disk_linux.c
        src/detection/dns/dns_linux.c
        src/detection/physicaldisk/physicaldisk_linux.c
        src/detection/physicalmemory/physicalmemory_linux.c
        src/detection/diskio/diskio_linux.c
        src/detection/displayserver/linux/displayserver_linux.c
        src/detection/displayserver/linux/drm.c
        src/detection/displayserver/linux/wayland/wayland.c
        src/detection/displayserver/linux/wayland/global-output.c
        src/detection/displayserver/linux/wayland/zwlr-output.c
        src/detection/displayserver/linux/wayland/kde-output.c
        src/detection/displayserver/linux/wayland/wlr-output-management-unstable-v1-protocol.c
        src/detection/displayserver/linux/wayland/kde-output-device-v2-protocol.c
        src/detection/displayserver/linux/wayland/kde-output-order-v1-protocol.c
        src/detection/displayserver/linux/wayland/xdg-output-unstable-v1-protocol.c
        src/detection/displayserver/linux/wmde.c
        src/detection/displayserver/linux/xcb.c
        src/detection/displayserver/linux/xlib.c
        src/detection/font/font_linux.c
        src/detection/gpu/gpu_linux.c
        src/detection/gpu/gpu_pci.c
        src/detection/gtk_qt/gtk.c
        src/detection/host/host_linux.c
        src/detection/icons/icons_linux.c
        src/detection/initsystem/initsystem_linux.c
        src/detection/libc/libc_linux.c
        src/detection/lm/lm_linux.c
        src/detection/loadavg/loadavg_linux.c
        src/detection/locale/locale_linux.c
        src/detection/localip/localip_linux.c
        src/detection/gamepad/gamepad_linux.c
        src/detection/media/media_linux.c
        src/detection/memory/memory_linux.c
        src/detection/monitor/monitor_linux.c
        src/detection/netio/netio_linux.c
        src/detection/opengl/opengl_linux.c
        src/detection/os/os_linux.c
        src/detection/packages/packages_linux.c
        src/detection/poweradapter/poweradapter_linux.c
        src/detection/processes/processes_linux.c
        src/detection/gtk_qt/qt.c
        src/detection/sound/sound_linux.c
        src/detection/swap/swap_linux.c
        src/detection/temps/temps_linux.c
        src/detection/terminalfont/terminalfont_linux.c
        src/detection/terminalshell/terminalshell_linux.c
        src/detection/terminalsize/terminalsize_linux.c
        src/detection/theme/theme_linux.c
        src/detection/uptime/uptime_linux.c
        src/detection/users/users_linux.c
        src/detection/wallpaper/wallpaper_linux.c
        src/detection/wifi/wifi_linux.c
        src/detection/wm/wm_nosupport.c
        src/detection/de/de_linux.c
        src/detection/wmtheme/wmtheme_linux.c
        src/detection/camera/camera_linux.c
        src/detection/zpool/zpool_linux.c
        src/util/platform/FFPlatform_unix.c
        src/util/binary_linux.c
    )
elseif(ANDROID)
    list(APPEND LIBFASTFETCH_SRC
        src/common/io/io_unix.c
        src/common/netif/netif_linux.c
        src/common/networking_linux.c
        src/common/processing_linux.c
        src/detection/battery/battery_android.c
        src/detection/bios/bios_android.c
        src/detection/bluetooth/bluetooth_nosupport.c
        src/detection/bluetoothradio/bluetoothradio_nosupport.c
        src/detection/board/board_android.c
        src/detection/bootmgr/bootmgr_nosupport.c
        src/detection/brightness/brightness_nosupport.c
        src/detection/chassis/chassis_nosupport.c
        src/detection/cpu/cpu_linux.c
        src/detection/cpucache/cpucache_linux.c
        src/detection/cursor/cursor_nosupport.c
        src/detection/cpuusage/cpuusage_linux.c
        src/detection/disk/disk_linux.c
        src/detection/dns/dns_linux.c
        src/detection/physicaldisk/physicaldisk_linux.c
        src/detection/physicalmemory/physicalmemory_nosupport.c
        src/detection/diskio/diskio_linux.c
        src/detection/displayserver/displayserver_android.c
        src/detection/font/font_nosupport.c
        src/detection/gpu/gpu_nosupport.c
        src/detection/host/host_android.c
        src/detection/icons/icons_nosupport.c
        src/detection/initsystem/initsystem_linux.c
        src/detection/libc/libc_android.c
        src/detection/lm/lm_nosupport.c
        src/detection/loadavg/loadavg_linux.c
        src/detection/locale/locale_linux.c
        src/detection/localip/localip_linux.c
        src/detection/gamepad/gamepad_nosupport.c
        src/detection/media/media_nosupport.c
        src/detection/memory/memory_linux.c
        src/detection/monitor/monitor_nosupport.c
        src/detection/netio/netio_linux.c
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
        src/detection/terminalsize/terminalsize_linux.c
        src/detection/theme/theme_nosupport.c
        src/detection/uptime/uptime_linux.c
        src/detection/users/users_linux.c
        src/detection/wallpaper/wallpaper_nosupport.c
        src/detection/wifi/wifi_android.c
        src/detection/wm/wm_nosupport.c
        src/detection/de/de_nosupport.c
        src/detection/wmtheme/wmtheme_nosupport.c
        src/detection/camera/camera_android.c
        src/detection/zpool/zpool_nosupport.c
        src/util/platform/FFPlatform_unix.c
        src/util/binary_linux.c
    )
elseif(FreeBSD)
    list(APPEND LIBFASTFETCH_SRC
        src/common/dbus.c
        src/common/io/io_unix.c
        src/common/netif/netif_bsd.c
        src/common/networking_linux.c
        src/common/processing_linux.c
        src/common/sysctl.c
        src/detection/battery/battery_bsd.c
        src/detection/bios/bios_bsd.c
        src/detection/bluetooth/bluetooth_linux.c
        src/detection/bluetoothradio/bluetoothradio_linux.c
        src/detection/board/board_bsd.c
        src/detection/bootmgr/bootmgr_bsd.c
        src/detection/brightness/brightness_bsd.c
        src/detection/chassis/chassis_bsd.c
        src/detection/cpu/cpu_bsd.c
        src/detection/cpucache/cpucache_shared.c
        src/detection/cpuusage/cpuusage_bsd.c
        src/detection/cursor/cursor_linux.c
        src/detection/disk/disk_bsd.c
        src/detection/dns/dns_linux.c
        src/detection/physicaldisk/physicaldisk_bsd.c
        src/detection/physicalmemory/physicalmemory_linux.c
        src/detection/diskio/diskio_bsd.c
        src/detection/displayserver/linux/displayserver_linux.c
        src/detection/displayserver/linux/drm.c
        src/detection/displayserver/linux/wayland/wayland.c
        src/detection/displayserver/linux/wayland/global-output.c
        src/detection/displayserver/linux/wayland/zwlr-output.c
        src/detection/displayserver/linux/wayland/kde-output.c
        src/detection/displayserver/linux/wayland/wlr-output-management-unstable-v1-protocol.c
        src/detection/displayserver/linux/wayland/kde-output-device-v2-protocol.c
        src/detection/displayserver/linux/wayland/kde-output-order-v1-protocol.c
        src/detection/displayserver/linux/wayland/xdg-output-unstable-v1-protocol.c
        src/detection/displayserver/linux/wmde.c
        src/detection/displayserver/linux/xcb.c
        src/detection/displayserver/linux/xlib.c
        src/detection/font/font_linux.c
        src/detection/gpu/gpu_bsd.c
        src/detection/gpu/gpu_pci.c
        src/detection/gtk_qt/gtk.c
        src/detection/host/host_bsd.c
        src/detection/lm/lm_linux.c
        src/detection/icons/icons_linux.c
        src/detection/initsystem/initsystem_linux.c
        src/detection/libc/libc_bsd.c
        src/detection/loadavg/loadavg_bsd.c
        src/detection/locale/locale_linux.c
        src/detection/localip/localip_linux.c
        src/detection/gamepad/gamepad_bsd.c
        src/detection/media/media_linux.c
        src/detection/memory/memory_bsd.c
        src/detection/monitor/monitor_linux.c
        src/detection/netio/netio_bsd.c
        src/detection/opengl/opengl_linux.c
        src/detection/os/os_linux.c
        src/detection/packages/packages_bsd.c
        src/detection/poweradapter/poweradapter_nosupport.c
        src/detection/processes/processes_bsd.c
        src/detection/gtk_qt/qt.c
        src/detection/sound/sound_bsd.c
        src/detection/swap/swap_bsd.c
        src/detection/temps/temps_bsd.c
        src/detection/terminalfont/terminalfont_linux.c
        src/detection/terminalshell/terminalshell_linux.c
        src/detection/terminalsize/terminalsize_linux.c
        src/detection/theme/theme_linux.c
        src/detection/uptime/uptime_bsd.c
        src/detection/users/users_linux.c
        src/detection/wallpaper/wallpaper_linux.c
        src/detection/wifi/wifi_bsd.c
        src/detection/wm/wm_nosupport.c
        src/detection/de/de_linux.c
        src/detection/wmtheme/wmtheme_linux.c
        src/detection/camera/camera_linux.c
        src/detection/zpool/zpool_linux.c
        src/util/platform/FFPlatform_unix.c
        src/util/binary_linux.c
    )
elseif(APPLE)
    list(APPEND LIBFASTFETCH_SRC
        src/common/io/io_unix.c
        src/common/netif/netif_bsd.c
        src/common/networking_linux.c
        src/common/processing_linux.c
        src/common/sysctl.c
        src/detection/battery/battery_apple.c
        src/detection/bios/bios_apple.c
        src/detection/bluetooth/bluetooth_apple.m
        src/detection/bluetoothradio/bluetoothradio_apple.m
        src/detection/board/board_apple.c
        src/detection/bootmgr/bootmgr_apple.c
        src/detection/brightness/brightness_apple.c
        src/detection/chassis/chassis_nosupport.c
        src/detection/cpu/cpu_apple.c
        src/detection/cpucache/cpucache_apple.c
        src/detection/cpuusage/cpuusage_apple.c
        src/detection/cursor/cursor_apple.m
        src/detection/disk/disk_bsd.c
        src/detection/dns/dns_linux.c
        src/detection/physicaldisk/physicaldisk_apple.c
        src/detection/physicalmemory/physicalmemory_apple.m
        src/detection/diskio/diskio_apple.c
        src/detection/displayserver/displayserver_apple.c
        src/detection/font/font_apple.m
        src/detection/gpu/gpu_apple.c
        src/detection/gpu/gpu_apple.m
        src/detection/host/host_apple.c
        src/detection/icons/icons_nosupport.c
        src/detection/initsystem/initsystem_linux.c
        src/detection/lm/lm_nosupport.c
        src/detection/loadavg/loadavg_bsd.c
        src/detection/libc/libc_apple.c
        src/detection/locale/locale_linux.c
        src/detection/localip/localip_linux.c
        src/detection/gamepad/gamepad_apple.c
        src/detection/media/media_apple.m
        src/detection/memory/memory_apple.c
        src/detection/monitor/monitor_apple.m
        src/detection/netio/netio_bsd.c
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
        src/detection/terminalsize/terminalsize_linux.c
        src/detection/theme/theme_nosupport.c
        src/detection/uptime/uptime_bsd.c
        src/detection/users/users_linux.c
        src/detection/wallpaper/wallpaper_apple.m
        src/detection/wifi/wifi_apple.m
        src/detection/wm/wm_apple.c
        src/detection/de/de_nosupport.c
        src/detection/wmtheme/wmtheme_apple.m
        src/detection/camera/camera_apple.m
        src/detection/zpool/zpool_nosupport.c
        src/util/apple/cf_helpers.c
        src/util/apple/osascript.m
        src/util/platform/FFPlatform_unix.c
        src/util/binary_apple.c
    )
elseif(WIN32)
    list(APPEND LIBFASTFETCH_SRC
        src/common/io/io_windows.c
        src/common/netif/netif_windows.c
        src/common/networking_windows.c
        src/common/processing_windows.c
        src/detection/battery/battery_windows.c
        src/detection/bios/bios_windows.c
        src/detection/bluetooth/bluetooth_windows.c
        src/detection/bluetoothradio/bluetoothradio_windows.c
        src/detection/board/board_windows.c
        src/detection/bootmgr/bootmgr_windows.c
        src/detection/brightness/brightness_windows.cpp
        src/detection/chassis/chassis_windows.c
        src/detection/cpu/cpu_windows.c
        src/detection/cpucache/cpucache_windows.c
        src/detection/cpuusage/cpuusage_windows.c
        src/detection/cursor/cursor_windows.c
        src/detection/disk/disk_windows.c
        src/detection/physicaldisk/physicaldisk_windows.c
        src/detection/diskio/diskio_windows.c
        src/detection/displayserver/displayserver_windows.c
        src/detection/dns/dns_windows.c
        src/detection/font/font_windows.c
        src/detection/gpu/gpu_windows.c
        src/detection/host/host_windows.c
        src/detection/icons/icons_windows.c
        src/detection/initsystem/initsystem_nosupport.c
        src/detection/libc/libc_windows.cpp
        src/detection/lm/lm_nosupport.c
        src/detection/loadavg/loadavg_nosupport.c
        src/detection/locale/locale_windows.c
        src/detection/localip/localip_windows.c
        src/detection/gamepad/gamepad_windows.c
        src/detection/media/media_windows.c
        src/detection/memory/memory_windows.c
        src/detection/physicalmemory/physicalmemory_linux.c
        src/detection/monitor/monitor_windows.c
        src/detection/netio/netio_windows.c
        src/detection/opengl/opengl_windows.c
        src/detection/os/os_windows.cpp
        src/detection/packages/packages_windows.c
        src/detection/poweradapter/poweradapter_nosupport.c
        src/detection/processes/processes_windows.c
        src/detection/sound/sound_windows.cpp
        src/detection/swap/swap_windows.c
        src/detection/terminalfont/terminalfont_windows.c
        src/detection/terminalshell/terminalshell_windows.c
        src/detection/terminalsize/terminalsize_windows.c
        src/detection/temps/temps_windows.cpp
        src/detection/theme/theme_nosupport.c
        src/detection/uptime/uptime_windows.c
        src/detection/users/users_windows.c
        src/detection/wallpaper/wallpaper_windows.c
        src/detection/wifi/wifi_windows.c
        src/detection/wm/wm_windows.cpp
        src/detection/de/de_nosupport.c
        src/detection/wmtheme/wmtheme_windows.c
        src/detection/camera/camera_windows.cpp
        src/detection/zpool/zpool_nosupport.c
        src/util/windows/getline.c
        src/util/windows/com.cpp
        src/util/windows/registry.c
        src/util/windows/unicode.c
        src/util/windows/wmi.cpp
        src/util/platform/FFPlatform_windows.c
        src/util/binary_windows.c
    )
elseif(SunOS)
    list(APPEND LIBFASTFETCH_SRC
        src/common/dbus.c
        src/common/io/io_unix.c
        src/common/netif/netif_bsd.c
        src/common/networking_linux.c
        src/common/processing_linux.c
        src/detection/battery/battery_nosupport.c
        src/detection/bios/bios_windows.c
        src/detection/board/board_windows.c
        src/detection/bootmgr/bootmgr_nosupport.c
        src/detection/brightness/brightness_nosupport.c
        src/detection/chassis/chassis_windows.c
        src/detection/cpu/cpu_sunos.c
        src/detection/cpucache/cpucache_shared.c
        src/detection/cpuusage/cpuusage_sunos.c
        src/detection/cursor/cursor_linux.c
        src/detection/bluetooth/bluetooth_nosupport.c
        src/detection/bluetoothradio/bluetoothradio_nosupport.c
        src/detection/disk/disk_sunos.c
        src/detection/dns/dns_linux.c
        src/detection/physicaldisk/physicaldisk_nosupport.c
        src/detection/physicalmemory/physicalmemory_linux.c
        src/detection/diskio/diskio_sunos.c
        src/detection/displayserver/linux/displayserver_linux.c
        src/detection/displayserver/linux/drm.c
        src/detection/displayserver/linux/wayland/wayland.c
        src/detection/displayserver/linux/wayland/global-output.c
        src/detection/displayserver/linux/wayland/zwlr-output.c
        src/detection/displayserver/linux/wayland/kde-output.c
        src/detection/displayserver/linux/wayland/wlr-output-management-unstable-v1-protocol.c
        src/detection/displayserver/linux/wayland/kde-output-device-v2-protocol.c
        src/detection/displayserver/linux/wayland/kde-output-order-v1-protocol.c
        src/detection/displayserver/linux/wayland/xdg-output-unstable-v1-protocol.c
        src/detection/displayserver/linux/wmde.c
        src/detection/displayserver/linux/xcb.c
        src/detection/displayserver/linux/xlib.c
        src/detection/font/font_linux.c
        src/detection/gpu/gpu_sunos.c
        src/detection/gpu/gpu_pci.c
        src/detection/gtk_qt/gtk.c
        src/detection/host/host_windows.c
        src/detection/icons/icons_linux.c
        src/detection/initsystem/initsystem_linux.c
        src/detection/libc/libc_nosupport.c
        src/detection/lm/lm_nosupport.c
        src/detection/loadavg/loadavg_sunos.c
        src/detection/locale/locale_linux.c
        src/detection/localip/localip_linux.c
        src/detection/gamepad/gamepad_nosupport.c
        src/detection/media/media_linux.c
        src/detection/memory/memory_sunos.c
        src/detection/monitor/monitor_linux.c
        src/detection/netio/netio_sunos.c
        src/detection/opengl/opengl_linux.c
        src/detection/os/os_sunos.c
        src/detection/packages/packages_sunos.c
        src/detection/poweradapter/poweradapter_nosupport.c
        src/detection/processes/processes_linux.c
        src/detection/gtk_qt/qt.c
        src/detection/sound/sound_linux.c
        src/detection/swap/swap_sunos.c
        src/detection/terminalfont/terminalfont_linux.c
        src/detection/terminalshell/terminalshell_linux.c
        src/detection/terminalsize/terminalsize_linux.c
        src/detection/theme/theme_linux.c
        src/detection/uptime/uptime_sunos.c
        src/detection/users/users_linux.c
        src/detection/wallpaper/wallpaper_linux.c
        src/detection/wifi/wifi_nosupport.c
        src/detection/wm/wm_nosupport.c
        src/detection/de/de_linux.c
        src/detection/wmtheme/wmtheme_linux.c
        src/detection/camera/camera_nosupport.c
        src/detection/zpool/zpool_linux.c
        src/util/platform/FFPlatform_unix.c
        src/util/binary_linux.c
    )
endif()

if(ENABLE_DIRECTX_HEADERS)
    message(STATUS "Enabling DirectX headers for WSL")
    list(APPEND LIBFASTFETCH_SRC src/detection/gpu/gpu_wsl.cpp)
endif()

# Proprietary GPU driver APIs
if(LINUX OR FreeBSD OR WIN32)
    list(APPEND LIBFASTFETCH_SRC src/detection/gpu/gpu_nvidia.c)
    list(APPEND LIBFASTFETCH_SRC src/detection/gpu/gpu_mthreads.c)
endif()
if(WIN32)
    list(APPEND LIBFASTFETCH_SRC src/detection/gpu/gpu_intel.c)
    list(APPEND LIBFASTFETCH_SRC src/detection/gpu/gpu_amd.c)
endif()
include(CheckFunctionExists)
check_function_exists(wcwidth HAVE_WCWIDTH)
if(NOT HAVE_WCWIDTH)
    list(APPEND LIBFASTFETCH_SRC src/3rdparty/mk_wcwidch/wcwidth.c)
endif()
if(LINUX)
    check_function_exists(statx HAVE_STATX)
endif()

if(ENABLE_SYSTEM_YYJSON)
    find_package(yyjson)
    if(yyjson_FOUND)
        message(STATUS "System provided yyjson is used")
    else()
        message(FATAL_ERROR "ENABLE_SYSTEM_YYJSON is set but system provided yyjson is not found")
    endif()
else()
    list(APPEND LIBFASTFETCH_SRC
        src/3rdparty/yyjson/yyjson.c
    )
endif()

add_library(libfastfetch OBJECT
    ${LIBFASTFETCH_SRC}
)

if(yyjson_FOUND)
    target_compile_definitions(libfastfetch PUBLIC FF_USE_SYSTEM_YYJSON)
    target_link_libraries(libfastfetch PUBLIC yyjson::yyjson)
    # `target_link_libraries(yyjson::yyjson)` sets rpath implicitly
else()
    # Used for dlopen finding dylibs installed by homebrew
    # `/opt/homebrew/lib` is not on in dlopen search path by default
    if(APPLE)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,/opt/homebrew/lib -Wl,-rpath,/usr/local/lib")
    endif()
endif()

if(LINUX AND EXISTS "/lib/ld-musl-${CMAKE_HOST_SYSTEM_PROCESSOR}.so.1")
    execute_process(COMMAND "/lib/ld-musl-${CMAKE_HOST_SYSTEM_PROCESSOR}.so.1"
                    ERROR_VARIABLE LD_MUSL_RESULT)
    if("${LD_MUSL_RESULT}" MATCHES "Version ([0-9]+\\.[0-9]+\\.[0-9]+)")
        target_compile_definitions(libfastfetch PUBLIC FF_MUSL_VERSION="${CMAKE_MATCH_1}")
    endif()
endif()
if(APPLE AND EXISTS "/usr/bin/otool")
    execute_process(COMMAND /usr/bin/otool -L /usr/bin/otool
                    OUTPUT_VARIABLE OTOOL_OTOOL_RESULT)
    if("${OTOOL_OTOOL_RESULT}" MATCHES "libSystem\\.B\\.dylib \\(.*current version ([0-9]+\\.[0-9]+\\.[0-9]+)\\)")
        target_compile_definitions(libfastfetch PUBLIC FF_LIBSYSTEM_VERSION="${CMAKE_MATCH_1}")
    endif()
endif()
if(FreeBSD AND EXISTS "/usr/local/bin/objdump")
    execute_process(COMMAND /bin/sh -c "/usr/local/bin/objdump -T /lib/libc.so.* | grep 'FBSD_[^ )]*' -o | sort -Vru | head -1"
                    OUTPUT_VARIABLE OBJDUMP_T_RESULT)
    if("${OBJDUMP_T_RESULT}" MATCHES "FBSD_([0-9]+\\.[0-9]+)")
        target_compile_definitions(libfastfetch PUBLIC FF_FBSD_VERSION="${CMAKE_MATCH_1}")
    endif()
endif()

target_compile_definitions(libfastfetch PUBLIC _GNU_SOURCE _XOPEN_SOURCE __STDC_WANT_LIB_EXT1__=1 _FILE_OFFSET_BITS=64)
if(WIN32)
    target_compile_definitions(libfastfetch PUBLIC WIN32_LEAN_AND_MEAN=1)
elseif(APPLE)
    target_compile_definitions(libfastfetch PUBLIC _DARWIN_C_SOURCE)
elseif(SunOS)
    target_compile_definitions(libfastfetch PUBLIC __EXTENSIONS__ _POSIX_C_SOURCE=1)
endif()

if(HAVE_STATX)
    target_compile_definitions(libfastfetch PUBLIC FF_HAVE_STATX)
endif()

if(HAVE_WCWIDTH)
    target_compile_definitions(libfastfetch PUBLIC FF_HAVE_WCWIDTH)
endif()

if(NOT "${CUSTOM_PCI_IDS_PATH}" STREQUAL "")
    message(STATUS "Custom file path of pci.ids: ${CUSTOM_PCI_IDS_PATH}")
    target_compile_definitions(libfastfetch PRIVATE FF_CUSTOM_PCI_IDS_PATH=${CUSTOM_PCI_IDS_PATH})
endif()
if(NOT "${CUSTOM_AMDGPU_IDS_PATH}" STREQUAL "")
    message(STATUS "Custom file path of amdgpu.ids: ${CUSTOM_AMDGPU_IDS_PATH}")
    target_compile_definitions(libfastfetch PRIVATE FF_CUSTOM_AMDGPU_IDS_PATH=${CUSTOM_AMDGPU_IDS_PATH})
endif()
if(NOT "${CUSTOM_OS_RELEASE_PATH}" STREQUAL "")
    message(STATUS "Custom file path of os_release: ${CUSTOM_OS_RELEASE_PATH}")
    target_compile_definitions(libfastfetch PRIVATE FF_CUSTOM_OS_RELEASE_PATH=${CUSTOM_OS_RELEASE_PATH})
endif()

if(NOT BINARY_LINK_TYPE STREQUAL "dlopen")
    message(STATUS "Enabling custom link type: ${BINARY_LINK_TYPE}")
    target_compile_definitions(libfastfetch PRIVATE FF_DISABLE_DLOPEN=1)
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

    if(NOT BINARY_LINK_TYPE STREQUAL "dlopen")
        target_link_directories(libfastfetch PUBLIC ${${VARNAME}_LIBRARY_DIRS})
        target_link_libraries(libfastfetch PRIVATE ${${VARNAME}_LIBRARIES})
    endif()

    foreach(FLAG ${${VARNAME}_CFLAGS_OTHER})
        if(FLAG MATCHES "-D.*")
            string(SUBSTRING ${FLAG} 2 -1 FLAG)
            target_compile_definitions(libfastfetch PRIVATE ${FLAG})
        endif()
    endforeach()
endfunction()

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
ff_lib_enable(DRM
    "libdrm"
    "Libdrm"
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
ff_lib_enable(FREETYPE
    "freetype2"
    "FreeType2"
)
ff_lib_enable(PULSE
    "libpulse"
    "Pulse"
)
ff_lib_enable(DDCUTIL
    "ddcutil"
    "Ddcutil"
)
ff_lib_enable(ELF
    "libelf"
    "libelf"
)
ff_lib_enable(DIRECTX_HEADERS
    "DirectX-Headers"
    "DirectX-Headers"
)
# The system <libzfs.h> is only usable on SunOS. We provide our local copy of it so it's always available.
if(ENABLE_LIBZFS)
    if(BINARY_LINK_TYPE STREQUAL "dlopen")
        target_compile_definitions(libfastfetch PRIVATE FF_HAVE_LIBZFS=1)
    else()
        set(CMAKE_REQUIRED_LIBRARIES_BACKUP ${CMAKE_REQUIRED_LIBRARIES})
        set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} zfs)
        check_function_exists("libzfs_init" LIBZFS_FOUND)
        set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES_BACKUP})
        if(NOT LIBZFS_FOUND)
            message(STATUS "Library: missing: LIBZFS")
        else()
            message(STATUS "Library: found LIBZFS")
            target_compile_definitions(libfastfetch PRIVATE FF_HAVE_LIBZFS=1)
            target_link_libraries(libfastfetch PRIVATE zfs)
        endif()
    endif()
endif()


if(ENABLE_THREADS)
    target_compile_definitions(libfastfetch PRIVATE FF_HAVE_THREADS)
    if(CMAKE_USE_PTHREADS_INIT) #Threads::Threads is not set for WIN32
        target_link_libraries(libfastfetch PRIVATE Threads::Threads)
    endif()
endif()

if(LINUX)
    target_link_libraries(libfastfetch
        PRIVATE "m"
    )
elseif(APPLE)
    target_link_libraries(libfastfetch
        PRIVATE "-framework AVFoundation"
        PRIVATE "-framework Cocoa"
        PRIVATE "-framework CoreFoundation"
        PRIVATE "-framework CoreAudio"
        PRIVATE "-framework CoreMedia"
        PRIVATE "-framework CoreVideo"
        PRIVATE "-framework CoreWLAN"
        PRIVATE "-framework IOBluetooth"
        PRIVATE "-framework IOKit"
        PRIVATE "-framework Metal"
        PRIVATE "-framework OpenGL"
        PRIVATE "-framework OpenCL"
        PRIVATE "-framework SystemConfiguration"
        PRIVATE "-weak_framework CoreDisplay"

        PRIVATE "-F /System/Library/PrivateFrameworks"
        PRIVATE "-weak_framework DisplayServices"
        PRIVATE "-weak_framework MediaRemote"
        PRIVATE "-weak_framework Apple80211"
    )
elseif(WIN32)
    target_compile_definitions(libfastfetch PRIVATE -D_WIN32_WINNT=0x0601)
    target_link_libraries(libfastfetch
        PRIVATE "dwmapi"
        PRIVATE "gdi32"
        PRIVATE "iphlpapi"
        PRIVATE "ole32"
        PRIVATE "oleaut32"
        PRIVATE "ws2_32"
        PRIVATE "ntdll"
        PRIVATE "version"
        PRIVATE "setupapi"
        PRIVATE "hid"
        PRIVATE "wtsapi32"
        PRIVATE "imagehlp"
    )
elseif(FreeBSD)
    target_link_libraries(libfastfetch
        PRIVATE "m"
        PRIVATE "usbhid"
        PRIVATE "geom"
    )
elseif(SunOS)
    target_link_libraries(libfastfetch
        PRIVATE "m"
        PRIVATE "socket"
        PRIVATE "kstat"
        PRIVATE "proc"
        PRIVATE "zfs"
        PRIVATE "nvpair"
    )
elseif(ANDROID)
    CHECK_LIBRARY_EXISTS(-l:libandroid-wordexp.a wordexp "" HAVE_LIBANDROID_WORDEXP_STATIC)
    if(HAVE_LIBANDROID_WORDEXP_STATIC)
        target_link_libraries(libfastfetch
            PRIVATE -l:libandroid-wordexp.a
        )
    else()
        CHECK_LIBRARY_EXISTS(android-wordexp wordexp "" HAVE_LIBANDROID_WORDEXP)
        if(HAVE_LIBANDROID_WORDEXP)
            target_link_libraries(libfastfetch
                PRIVATE android-wordexp
            )
        endif()
    endif()
endif()

target_include_directories(libfastfetch
    PUBLIC ${PROJECT_BINARY_DIR}
    PUBLIC ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(libfastfetch
    PRIVATE ${CMAKE_DL_LIBS}
)

target_compile_options(libfastfetch PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions -fno-rtti>)

if(WIN32)
    set(CMAKE_CXX_STANDARD 20)
    include(CheckIncludeFileCXX)
    CHECK_INCLUDE_FILE_CXX("winrt/Windows.Foundation.h" HAVE_WINRT)
    if(HAVE_WINRT)
        add_library(ffwinrt MODULE src/detection/media/media_windows.dll.cpp)
        target_link_libraries(ffwinrt PRIVATE "RuntimeObject")
        target_compile_definitions(ffwinrt PRIVATE WIN32_LEAN_AND_MEAN=1 WINRT_LEAN_AND_MEAN=1)
        target_link_options(ffwinrt
            PRIVATE "-static" # stdc++, winpthread, gcc_s, etc.
        )
    endif()
    set(CMAKE_CXX_STANDARD 17)
endif()
if(FreeBSD)
  set(CMAKE_REQUIRED_INCLUDES "/usr/local/include" "/usr/include")
endif()
if(LINUX OR FreeBSD)
    CHECK_INCLUDE_FILE("linux/videodev2.h" HAVE_LINUX_VIDEODEV2)
    if(HAVE_LINUX_VIDEODEV2)
        target_compile_definitions(libfastfetch PRIVATE FF_HAVE_LINUX_VIDEODEV2=1)
    endif()
endif()
if(LINUX)
    CHECK_INCLUDE_FILE("linux/wireless.h" HAVE_LINUX_WIRELESS)
    if(HAVE_LINUX_WIRELESS)
        target_compile_definitions(libfastfetch PRIVATE FF_HAVE_LINUX_WIRELESS=1)
    endif()
endif()
if(NOT WIN32)
    CHECK_INCLUDE_FILE("utmpx.h" HAVE_UTMPX)
    if(HAVE_UTMPX)
        target_compile_definitions(libfastfetch PRIVATE FF_HAVE_UTMPX=1)
    endif()
    CHECK_INCLUDE_FILE("wordexp.h" HAVE_WORDEXP)
    if(HAVE_WORDEXP)
        target_compile_definitions(libfastfetch PRIVATE FF_HAVE_WORDEXP=1)
    endif()
    if(ENABLE_THREADS AND CMAKE_USE_PTHREADS_INIT)
        CHECK_INCLUDE_FILE("pthread_np.h" HAVE_PTHREAD_NP)
        if(HAVE_PTHREAD_NP)
            target_compile_definitions(libfastfetch PRIVATE FF_HAVE_PTHREAD_NP=1)
            set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} pthread_np.h)
        endif()
        set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} Threads::Threads)
        check_function_exists("pthread_timedjoin_np" HAVE_TIMEDJOIN_NP)
        if(HAVE_TIMEDJOIN_NP)
            target_compile_definitions(libfastfetch PRIVATE FF_HAVE_TIMEDJOIN_NP=1)
        else()
            message(WARNING "pthread_timedjoin_np was not found; networking timeout will not work")
        endif()
    endif()
endif()

set(PACKAGES_DISABLE_LIST "")
foreach(package_manager ${PACKAGE_MANAGERS})
    if(PACKAGES_DISABLE_${package_manager})
        list(APPEND PACKAGES_DISABLE_LIST "${package_manager}")
    endif()
endforeach()
if("${PACKAGES_DISABLE_LIST}" STREQUAL "")
    set(PACKAGES_DISABLE_LIST "FF_PACKAGES_FLAG_NONE")
else()
    message(STATUS "Disabled package managers: ${PACKAGES_DISABLE_LIST}")
    list(TRANSFORM PACKAGES_DISABLE_LIST PREPEND "FF_PACKAGES_FLAG_")
    list(TRANSFORM PACKAGES_DISABLE_LIST APPEND "_BIT")
    list(JOIN PACKAGES_DISABLE_LIST " | " PACKAGES_DISABLE_LIST)
endif()
target_compile_definitions(libfastfetch PRIVATE FF_PACKAGES_DISABLE_LIST=${PACKAGES_DISABLE_LIST})

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

# Prevent fastfetch from linking to libstdc++
set(CMAKE_CXX_IMPLICIT_LINK_LIBRARIES "")
set(CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES "")
set_target_properties(fastfetch PROPERTIES LINKER_LANGUAGE C)
set_target_properties(flashfetch PROPERTIES LINKER_LANGUAGE C)

if(WIN32)
    target_sources(fastfetch
        PRIVATE src/util/windows/version.rc
    )
    target_sources(flashfetch
        PRIVATE src/util/windows/version.rc
    )
elseif(APPLE)
    target_link_options(fastfetch
        PRIVATE LINKER:-sectcreate,__TEXT,__info_plist,Info.plist
    )
    target_link_options(flashfetch
        PRIVATE LINKER:-sectcreate,__TEXT,__info_plist,Info.plist
    )
endif()

if(BINARY_LINK_TYPE STREQUAL "static")
    target_link_options(fastfetch PRIVATE "-static")
    target_link_options(flashfetch PRIVATE "-static")
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

    add_executable(fastfetch-test-format
        tests/format.c
    )
    target_link_libraries(fastfetch-test-format
        PRIVATE libfastfetch
    )

    enable_testing()
    add_test(NAME test-strbuf COMMAND fastfetch-test-strbuf)
    add_test(NAME test-list COMMAND fastfetch-test-list)
    add_test(NAME test-format COMMAND fastfetch-test-format)
endif()

##################
# install target #
##################

include(GNUInstallDirs)

install(
    TARGETS fastfetch flashfetch
    DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

if (TARGET ffwinrt)
    install(
        TARGETS ffwinrt
        DESTINATION "${CMAKE_INSTALL_BINDIR}"
    )
endif()

install(
    FILES "${CMAKE_SOURCE_DIR}/completions/${CMAKE_PROJECT_NAME}.bash"
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/bash-completion/completions"
    RENAME "${CMAKE_PROJECT_NAME}"
)

install(
    FILES "${CMAKE_SOURCE_DIR}/completions/${CMAKE_PROJECT_NAME}.zsh"
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/zsh/site-functions"
    RENAME "_${CMAKE_PROJECT_NAME}"
)

install(
    FILES "${CMAKE_SOURCE_DIR}/completions/${CMAKE_PROJECT_NAME}.fish"
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/fish/vendor_completions.d"
    RENAME "${CMAKE_PROJECT_NAME}.fish"
)

install(
    DIRECTORY "${CMAKE_SOURCE_DIR}/presets"
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/${CMAKE_PROJECT_NAME}"
)

if(INSTALL_LICENSE)
    install(
        FILES "${CMAKE_SOURCE_DIR}/LICENSE"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/licenses/${CMAKE_PROJECT_NAME}"
    )
endif()

install(
    FILES "${PROJECT_BINARY_DIR}/fastfetch.1"
    DESTINATION "${CMAKE_INSTALL_MANDIR}/man1"
)

##################
# package target #
##################

set(CPACK_GENERATOR "TGZ;ZIP")
if(APPLE)
    string(TOLOWER "${CMAKE_PROJECT_NAME}-macos-universal" CPACK_PACKAGE_FILE_NAME)
else() # We don't use this in Windows
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
        set(CMAKE_SYSTEM_PROCESSOR "amd64")
    endif()
    if(IS_MUSL)
        string(TOLOWER "${CMAKE_PROJECT_NAME}-musl-${CMAKE_SYSTEM_PROCESSOR}" CPACK_PACKAGE_FILE_NAME)
    else()
        string(TOLOWER "${CMAKE_PROJECT_NAME}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}" CPACK_PACKAGE_FILE_NAME)
    endif()
endif()

if(LINUX)
    find_program(HAVE_DPKG "dpkg")
    if(HAVE_DPKG)
        set(CPACK_GENERATOR "${CPACK_GENERATOR};DEB")

        set(CPACK_DEBIAN_PACKAGE_SECTION, "utils")
        set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")

        if(NOT IS_MUSL)
            EXECUTE_PROCESS(
                COMMAND getconf GNU_LIBC_VERSION
                OUTPUT_VARIABLE GLIBC_VERSION
                OUTPUT_STRIP_TRAILING_WHITESPACE)
            if(GLIBC_VERSION)
                STRING(REPLACE "glibc " "" GLIBC_VERSION ${GLIBC_VERSION})
                set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= ${GLIBC_VERSION})")
                message(STATUS "found glibc ${GLIBC_VERSION}")
            else()
                message(WARNING "Could not determine glibc version. If `musl` is used, `-DIS_MUSL=ON` should be set")
            endif()
        endif()
    endif()

    find_program(HAVE_RPMBUILD "rpmbuild")
    if(HAVE_RPMBUILD)
        set(CPACK_GENERATOR "${CPACK_GENERATOR};RPM")

        set(CPACK_RPM_PACKAGE_LICENSE "MIT")
    endif()
endif()

set(CPACK_SET_DESTDIR ON)

set(CPACK_PACKAGE_CONTACT "Linus Dierheimer <Linus@Dierheimer.de>")
set(CPACK_PACKAGE_DESCRIPTION "\
fastfetch is a neofetch-like tool for fetching system information and displaying them in a pretty way. \
It is written mostly in C to achieve much better performance.\
")

include(CPack)

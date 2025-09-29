cmake_minimum_required(VERSION 3.12.0) # target_link_libraries with OBJECT libs & project homepage url

project(fastfetch
    VERSION 2.53.0
    LANGUAGES C
    DESCRIPTION "Fast neofetch-like system information tool"
    HOMEPAGE_URL "https://github.com/fastfetch-cli/fastfetch"
)

set(PROJECT_LICENSE "MIT license")

if(DEFINED CMAKE_SYSTEM_PROCESSOR_OVERRIDE) # Used by github actions for i686 build
    set(CMAKE_SYSTEM_PROCESSOR ${CMAKE_SYSTEM_PROCESSOR_OVERRIDE} CACHE INTERNAL "")
endif()
string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" CMAKE_SYSTEM_PROCESSOR)
if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    set(CMAKE_SYSTEM_PROCESSOR "amd64")
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
    set(CMAKE_SYSTEM_PROCESSOR "aarch64")
endif()
message(STATUS "Build for system processor: ${CMAKE_SYSTEM_PROCESSOR}")

###################
# Target Platform #
###################
if(ANDROID)
    set(LINUX FALSE)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    set(LINUX TRUE CACHE BOOL "..." FORCE) # LINUX means GNU/Linux, not just the kernel
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "FreeBSD")
    set(FreeBSD TRUE CACHE BOOL "..." FORCE)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "OpenBSD")
    set(OpenBSD TRUE CACHE BOOL "..." FORCE)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "MidnightBSD")
    set(FreeBSD TRUE CACHE BOOL "..." FORCE)
    set(MidnightBSD TRUE CACHE BOOL "..." FORCE)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "NetBSD")
    set(NetBSD TRUE CACHE BOOL "..." FORCE)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "DragonFly")
    set(FreeBSD TRUE CACHE BOOL "..." FORCE)
    set(DragonFly TRUE CACHE BOOL "..." FORCE)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "SunOS")
    set(SunOS TRUE CACHE BOOL "..." FORCE)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Haiku")
    set(Haiku TRUE CACHE BOOL "..." FORCE)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "GNU")
    set(GNU TRUE CACHE BOOL "..." FORCE)
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

cmake_dependent_option(ENABLE_VULKAN "Enable vulkan" ON "LINUX OR APPLE OR FreeBSD OR OpenBSD OR NetBSD OR WIN32 OR ANDROID OR SunOS OR Haiku OR GNU" OFF)
cmake_dependent_option(ENABLE_WAYLAND "Enable wayland-client" ON "LINUX OR FreeBSD OR OpenBSD OR NetBSD OR GNU" OFF)
cmake_dependent_option(ENABLE_XCB_RANDR "Enable xcb-randr" ON "LINUX OR FreeBSD OR OpenBSD OR NetBSD OR ANDROID OR SunOS OR GNU" OFF)
cmake_dependent_option(ENABLE_XRANDR "Enable xrandr" ON "LINUX OR FreeBSD OR OpenBSD OR NetBSD OR ANDROID OR SunOS OR GNU" OFF)
cmake_dependent_option(ENABLE_DRM "Enable libdrm" ON "LINUX OR FreeBSD OR OpenBSD OR NetBSD OR SunOS OR GNU" OFF)
cmake_dependent_option(ENABLE_DRM_AMDGPU "Enable libdrm_amdgpu" ON "LINUX OR FreeBSD OR GNU" OFF)
cmake_dependent_option(ENABLE_GIO "Enable gio-2.0" ON "LINUX OR FreeBSD OR OpenBSD OR NetBSD OR ANDROID OR SunOS OR GNU" OFF)
cmake_dependent_option(ENABLE_DCONF "Enable dconf" ON "LINUX OR FreeBSD OR OpenBSD OR NetBSD OR ANDROID OR SunOS OR GNU" OFF)
cmake_dependent_option(ENABLE_DBUS "Enable dbus-1" ON "LINUX OR FreeBSD OR OpenBSD OR NetBSD OR ANDROID OR SunOS OR Haiku OR GNU" OFF)
cmake_dependent_option(ENABLE_SQLITE3 "Enable sqlite3" ON "LINUX OR FreeBSD OR APPLE OR OpenBSD OR NetBSD OR SunOS OR GNU" OFF)
cmake_dependent_option(ENABLE_RPM "Enable rpm" ON "LINUX OR GNU" OFF)
cmake_dependent_option(ENABLE_IMAGEMAGICK7 "Enable imagemagick 7" ON "LINUX OR FreeBSD OR OpenBSD OR NetBSD OR APPLE OR ANDROID OR WIN32 OR SunOS OR Haiku OR GNU" OFF)
cmake_dependent_option(ENABLE_IMAGEMAGICK6 "Enable imagemagick 6" ON "LINUX OR FreeBSD OR OpenBSD OR NetBSD OR APPLE OR SunOS OR GNU" OFF)
cmake_dependent_option(ENABLE_CHAFA "Enable chafa" ON "ENABLE_IMAGEMAGICK6 OR ENABLE_IMAGEMAGICK7" OFF)
cmake_dependent_option(ENABLE_EGL "Enable egl" ON "LINUX OR FreeBSD OR OpenBSD OR NetBSD OR ANDROID OR WIN32 OR SunOS OR Haiku OR GNU" OFF)
cmake_dependent_option(ENABLE_GLX "Enable glx" ON "LINUX OR FreeBSD OR OpenBSD OR NetBSD OR ANDROID OR SunOS OR GNU" OFF)
cmake_dependent_option(ENABLE_OPENCL "Enable opencl" ON "LINUX OR FreeBSD OR OpenBSD OR NetBSD OR WIN32 OR ANDROID OR SunOS OR Haiku OR GNU" OFF)
cmake_dependent_option(ENABLE_FREETYPE "Enable freetype" ON "ANDROID" OFF)
cmake_dependent_option(ENABLE_PULSE "Enable pulse" ON "LINUX OR ANDROID OR GNU" OFF)
cmake_dependent_option(ENABLE_DDCUTIL "Enable ddcutil" ON "LINUX" OFF)
cmake_dependent_option(ENABLE_DIRECTX_HEADERS "Enable DirectX headers for WSL" ON "LINUX" OFF)
cmake_dependent_option(ENABLE_ELF "Enable libelf" ON "LINUX OR ANDROID OR DragonFly OR Haiku OR GNU" OFF)
cmake_dependent_option(ENABLE_THREADS "Enable multithreading" ON "Threads_FOUND" OFF)
cmake_dependent_option(ENABLE_LIBZFS "Enable libzfs" ON "LINUX OR FreeBSD OR SunOS" OFF)
cmake_dependent_option(ENABLE_PCIACCESS "Enable libpciaccess" ON "NetBSD OR OpenBSD" OFF)

option(ENABLE_ZLIB "Enable zlib" ON)
option(ENABLE_SYSTEM_YYJSON "Use system provided (instead of fastfetch embedded) yyjson library" OFF)
option(ENABLE_ASAN "Build fastfetch with ASAN (address sanitizer)" OFF)
option(ENABLE_LTO "Enable link-time optimization in release mode if supported" ON)
option(BUILD_FLASHFETCH "Build flashfetch" ON) # Also build the flashfetch binary
option(BUILD_TESTS "Build tests" OFF) # Also create test executables
option(SET_TWEAK "Add tweak to project version" ON) # This is set to off by github actions for release builds
option(IS_MUSL "Build with musl libc" OFF) # Used by Github Actions
option(INSTALL_LICENSE "Install license into /usr/share/licenses" ON)
option(ENABLE_EMBEDDED_PCIIDS "Embed pci.ids into fastfetch, requires `python`" OFF)
option(ENABLE_EMBEDDED_AMDGPUIDS "Embed amdgpu.ids into fastfetch, requires `python`" OFF)

set(BINARY_LINK_TYPE_OPTIONS dlopen dynamic static)
set(BINARY_LINK_TYPE dlopen CACHE STRING "How to link fastfetch")
set_property(CACHE BINARY_LINK_TYPE PROPERTY STRINGS ${BINARY_LINK_TYPE_OPTIONS})
if(NOT BINARY_LINK_TYPE IN_LIST BINARY_LINK_TYPE_OPTIONS)
    message(FATAL_ERROR "BINARY_LINK_TYPE must be one of ${BINARY_LINK_TYPE_OPTIONS}")
endif()

set(PACKAGE_MANAGERS AM APK BREW CHOCO DPKG EMERGE EOPKG FLATPAK GUIX LINGLONG LPKG LPKGBUILD MACPORTS NIX OPKG PACMAN PACSTALL PALUDIS PISI PKG PKGTOOL RPM SCOOP SNAP SOAR SORCERY WINGET XBPS)
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

set(WARNING_FLAGS "-Wall -Wextra -Wconversion -Werror=uninitialized -Werror=return-type -Werror=vla")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARNING_FLAGS} -Werror=incompatible-pointer-types -Werror=implicit-function-declaration -Werror=int-conversion")

if(WIN32 OR Haiku OR ENABLE_DIRECTX_HEADERS)
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
    message(STATUS "Minifying 'help.json'")
    execute_process(COMMAND ${Python_EXECUTABLE} -c "import json,sys;json.dump(json.load(sys.stdin),sys.stdout,separators=(',',':'))"
                    INPUT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/src/data/help.json"
                    OUTPUT_VARIABLE DATATEXT_JSON_HELP)
    if(DATATEXT_JSON_HELP STREQUAL "")
        message(ERROR "DATATEXT_JSON_HELP is empty, which should not happen!")
    endif()
else()
    message(WARNING "Python3 is not found, 'help.json' will not be minified")
    file(READ "src/data/help.json" DATATEXT_JSON_HELP)
endif()

if(ENABLE_EMBEDDED_PCIIDS AND NOT EXISTS "${PROJECT_BINARY_DIR}/fastfetch_pciids.c.inc")
    if(Python_FOUND)
        if(NOT EXISTS "${PROJECT_BINARY_DIR}/pci.ids")
            message(STATUS "'${PROJECT_BINARY_DIR}/pci.ids' is missing, downloading...")
            file(DOWNLOAD "https://pci-ids.ucw.cz/v2.2/pci.ids" "${PROJECT_BINARY_DIR}/pci.ids")
        endif()
        message(STATUS "Generating 'fastfetch_pciids.c.inc'")
        execute_process(COMMAND ${Python_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/scripts/gen-pciids.py" "${PROJECT_BINARY_DIR}/pci.ids"
                        OUTPUT_FILE "${PROJECT_BINARY_DIR}/fastfetch_pciids.c.inc"
                        RESULT_VARIABLE PYTHON_PCIIDS_RETCODE)
        if(NOT PYTHON_PCIIDS_RETCODE EQUAL 0)
            file(REMOVE "${PROJECT_BINARY_DIR}/fastfetch_pciids.c.inc")
            message(FATAL_ERROR "Failed to generate 'fastfetch_pciids.c.inc'")
        endif()
    else()
        message(WARNING "Python3 is not found, 'fastfetch_pciids.c.inc' will not be generated")
        set(ENABLE_EMBEDDED_PCIIDS OFF)
    endif()
endif()

if(ENABLE_EMBEDDED_AMDGPUIDS AND NOT EXISTS "${PROJECT_BINARY_DIR}/fastfetch_amdgpuids.c.inc")
    if(Python_FOUND)
        if(NOT EXISTS "${PROJECT_BINARY_DIR}/amdgpu.ids")
            message(STATUS "'${PROJECT_BINARY_DIR}/amdgpu.ids' is missing, downloading...")
            file(DOWNLOAD "https://gitlab.freedesktop.org/mesa/drm/-/raw/main/data/amdgpu.ids" "${PROJECT_BINARY_DIR}/amdgpu.ids")
        endif()
        message(STATUS "Generating 'fastfetch_amdgpuids.c.inc'")
        execute_process(COMMAND ${Python_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/scripts/gen-amdgpuids.py" "${PROJECT_BINARY_DIR}/amdgpu.ids"
                        OUTPUT_FILE "${PROJECT_BINARY_DIR}/fastfetch_amdgpuids.c.inc"
                        RESULT_VARIABLE PYTHON_AMDGPUIDS_RETCODE)
        if(NOT PYTHON_AMDGPUIDS_RETCODE EQUAL 0)
            file(REMOVE "${PROJECT_BINARY_DIR}/fastfetch_amdgpuids.c.inc")
            message(FATAL_ERROR "Failed to generate 'fastfetch_amdgpuids.c.inc'")
        endif()
    else()
        message(WARNING "Python3 is not found, 'fastfetch_amdgpuids.c.inc' will not be generated")
        set(ENABLE_EMBEDDED_AMDGPUIDS OFF)
    endif()
endif()

if(Python_FOUND)
    message(STATUS "Generating 'fastfetch.1'")
    execute_process(COMMAND ${Python_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/scripts/gen-man.py"
                    OUTPUT_FILE "${PROJECT_BINARY_DIR}/fastfetch.1"
                    RESULT_VARIABLE PYTHON_MANPAGE_RETCODE)
        if(NOT PYTHON_MANPAGE_RETCODE EQUAL 0)
            message(FATAL_ERROR "Failed to generate 'fastfetch.1'")
        endif()
else()
    message(WARNING "Python3 is not found, use basic 'fastfetch.1.in' instead")
    string(TIMESTAMP FASTFETCH_BUILD_DATE "%d %B %Y" UTC)
    configure_file(doc/fastfetch.1.in fastfetch.1 @ONLY)
endif()

fastfetch_encode_c_string("${DATATEXT_JSON_HELP}" DATATEXT_JSON_HELP)
fastfetch_load_text(src/data/structure.txt DATATEXT_STRUCTURE)

configure_file(src/fastfetch_config.h.in fastfetch_config.h @ONLY)
configure_file(src/fastfetch_datatext.h.in fastfetch_datatext.h @ONLY)
if(APPLE)
    configure_file(src/util/apple/Info.plist.in Info.plist @ONLY)
endif()

####################
# Ascii image data #
####################

file(GLOB LOGO_FILES CONFIGURE_DEPENDS "src/logo/ascii/*.txt")
set(LOGO_BUILTIN_H "#pragma once\n#pragma GCC diagnostic ignored \"-Wtrigraphs\"\n\n")
foreach(file ${LOGO_FILES})
    fastfetch_load_text("${file}" content)
    get_filename_component(file "${file}" NAME_WE)
    string(TOUPPER "${file}" file)
    set(LOGO_BUILTIN_H "${LOGO_BUILTIN_H}#define FASTFETCH_DATATEXT_LOGO_${file} ${content}\n")
endforeach()
file(GENERATE OUTPUT logo_builtin.h CONTENT "${LOGO_BUILTIN_H}")

#######################
# libfastfetch target #
#######################

set(LIBFASTFETCH_SRC
    src/common/commandoption.c
    src/common/duration.c
    src/common/font.c
    src/common/format.c
    src/common/frequency.c
    src/common/init.c
    src/common/jsonconfig.c
    src/common/library.c
    src/common/modules.c
    src/common/netif/netif.c
    src/common/networking/networking_common.c
    src/common/option.c
    src/common/parsing.c
    src/common/percent.c
    src/common/printing.c
    src/common/properties.c
    src/common/settings.c
    src/common/size.c
    src/common/temps.c
    src/common/time.c
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
    src/modules/btrfs/btrfs.c
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
    src/modules/keyboard/keyboard.c
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
    src/modules/mouse/mouse.c
    src/modules/terminal/terminal.c
    src/modules/terminaltheme/terminaltheme.c
    src/modules/terminalfont/terminalfont.c
    src/modules/terminalsize/terminalsize.c
    src/modules/theme/theme.c
    src/modules/title/title.c
    src/modules/tpm/tpm.c
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
    src/options/logo.c
    src/options/general.c
    src/util/edidHelper.c
    src/util/base64.c
    src/util/FFlist.c
    src/util/FFstrbuf.c
    src/util/kmod.c
    src/util/path.c
    src/util/platform/FFPlatform.c
    src/util/smbiosHelper.c
)

if(LINUX)
    list(APPEND LIBFASTFETCH_SRC
        src/common/dbus.c
        src/common/io/io_unix.c
        src/common/netif/netif_linux.c
        src/common/networking/networking_linux.c
        src/common/processing_linux.c
        src/detection/battery/battery_linux.c
        src/detection/bios/bios_linux.c
        src/detection/board/board_linux.c
        src/detection/bootmgr/bootmgr_linux.c
        src/detection/brightness/brightness_linux.c
        src/detection/btrfs/btrfs_linux.c
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
        src/detection/displayserver/linux/common.c
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
        src/detection/gpu/gpu_drm.c
        src/detection/gpu/gpu_pci.c
        src/detection/gtk_qt/gtk.c
        src/detection/host/host_linux.c
        src/detection/host/host_mac.c
        src/detection/icons/icons_linux.c
        src/detection/initsystem/initsystem_linux.c
        src/detection/keyboard/keyboard_linux.c
        src/detection/libc/libc_linux.c
        src/detection/lm/lm_linux.c
        src/detection/loadavg/loadavg_linux.c
        src/detection/locale/locale_linux.c
        src/detection/localip/localip_linux.c
        src/detection/gamepad/gamepad_linux.c
        src/detection/media/media_linux.c
        src/detection/memory/memory_linux.c
        src/detection/mouse/mouse_linux.c
        src/detection/netio/netio_linux.c
        src/detection/opengl/opengl_linux.c
        src/detection/os/os_linux.c
        src/detection/packages/packages_linux.c
        src/detection/packages/packages_nix.c
        src/detection/poweradapter/poweradapter_linux.c
        src/detection/processes/processes_linux.c
        src/detection/gtk_qt/qt.c
        src/detection/sound/sound_linux.c
        src/detection/swap/swap_linux.c
        src/detection/terminalfont/terminalfont_linux.c
        src/detection/terminalshell/terminalshell_linux.c
        src/detection/terminalsize/terminalsize_linux.c
        src/detection/theme/theme_linux.c
        src/detection/tpm/tpm_linux.c
        src/detection/uptime/uptime_linux.c
        src/detection/users/users_linux.c
        src/detection/wallpaper/wallpaper_linux.c
        src/detection/wifi/wifi_linux.c
        src/detection/wm/wm_linux.c
        src/detection/de/de_linux.c
        src/detection/wmtheme/wmtheme_linux.c
        src/detection/camera/camera_linux.c
        src/detection/zpool/zpool_linux.c
        src/util/platform/FFPlatform_unix.c
        src/util/binary_linux.c
    )
elseif(ANDROID)
    list(APPEND LIBFASTFETCH_SRC
        src/common/dbus.c
        src/common/io/io_unix.c
        src/common/netif/netif_linux.c
        src/common/networking/networking_linux.c
        src/common/processing_linux.c
        src/detection/battery/battery_android.c
        src/detection/bios/bios_android.c
        src/detection/bluetooth/bluetooth_nosupport.c
        src/detection/bluetoothradio/bluetoothradio_nosupport.c
        src/detection/board/board_android.c
        src/detection/bootmgr/bootmgr_nosupport.c
        src/detection/brightness/brightness_nosupport.c
        src/detection/btrfs/btrfs_nosupport.c
        src/detection/chassis/chassis_nosupport.c
        src/detection/cpu/cpu_linux.c
        src/detection/cpucache/cpucache_linux.c
        src/detection/cursor/cursor_linux.c
        src/detection/cpuusage/cpuusage_linux.c
        src/detection/disk/disk_linux.c
        src/detection/dns/dns_linux.c
        src/detection/physicaldisk/physicaldisk_linux.c
        src/detection/physicalmemory/physicalmemory_nosupport.c
        src/detection/diskio/diskio_linux.c
        src/detection/displayserver/displayserver_android.c
        src/detection/displayserver/linux/common.c
        src/detection/displayserver/linux/wmde.c
        src/detection/displayserver/linux/xcb.c
        src/detection/displayserver/linux/xlib.c
        src/detection/gtk_qt/gtk.c
        src/detection/gtk_qt/qt.c
        src/detection/font/font_linux.c
        src/detection/gpu/gpu_android.c
        src/detection/host/host_android.c
        src/detection/icons/icons_linux.c
        src/detection/initsystem/initsystem_linux.c
        src/detection/keyboard/keyboard_nosupport.c
        src/detection/libc/libc_android.c
        src/detection/lm/lm_nosupport.c
        src/detection/loadavg/loadavg_linux.c
        src/detection/locale/locale_linux.c
        src/detection/localip/localip_linux.c
        src/detection/gamepad/gamepad_nosupport.c
        src/detection/media/media_linux.c
        src/detection/memory/memory_linux.c
        src/detection/mouse/mouse_nosupport.c
        src/detection/netio/netio_linux.c
        src/detection/opengl/opengl_linux.c
        src/detection/os/os_android.c
        src/detection/packages/packages_linux.c
        src/detection/packages/packages_nix.c
        src/detection/poweradapter/poweradapter_nosupport.c
        src/detection/processes/processes_linux.c
        src/detection/sound/sound_linux.c
        src/detection/swap/swap_linux.c
        src/detection/terminalfont/terminalfont_android.c
        src/detection/terminalfont/terminalfont_linux.c
        src/detection/terminalshell/terminalshell_linux.c
        src/detection/terminalsize/terminalsize_linux.c
        src/detection/theme/theme_linux.c
        src/detection/tpm/tpm_nosupport.c
        src/detection/uptime/uptime_linux.c
        src/detection/users/users_linux.c
        src/detection/wallpaper/wallpaper_linux.c
        src/detection/wifi/wifi_android.c
        src/detection/wm/wm_linux.c
        src/detection/de/de_linux.c
        src/detection/wmtheme/wmtheme_linux.c
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
        src/common/networking/networking_linux.c
        src/common/processing_linux.c
        src/common/sysctl.c
        src/detection/battery/battery_bsd.c
        src/detection/bios/bios_bsd.c
        src/detection/bluetoothradio/bluetoothradio_nosupport.c
        src/detection/board/board_bsd.c
        src/detection/bootmgr/bootmgr_bsd.c
        src/detection/brightness/brightness_bsd.c
        src/detection/btrfs/btrfs_nosupport.c
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
        src/detection/displayserver/linux/common.c
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
        src/detection/gpu/gpu_drm.c
        src/detection/gpu/gpu_pci.c
        src/detection/gtk_qt/gtk.c
        src/detection/host/host_bsd.c
        src/detection/host/host_mac.c
        src/detection/lm/lm_linux.c
        src/detection/icons/icons_linux.c
        src/detection/initsystem/initsystem_linux.c
        src/detection/keyboard/keyboard_bsd.c
        src/detection/libc/libc_bsd.c
        src/detection/loadavg/loadavg_bsd.c
        src/detection/locale/locale_linux.c
        src/detection/localip/localip_linux.c
        src/detection/gamepad/gamepad_bsd.c
        src/detection/media/media_linux.c
        src/detection/memory/memory_bsd.c
        src/detection/mouse/mouse_bsd.c
        src/detection/netio/netio_bsd.c
        src/detection/opengl/opengl_linux.c
        src/detection/os/os_linux.c
        src/detection/packages/packages_bsd.c
        src/detection/poweradapter/poweradapter_nosupport.c
        src/detection/processes/processes_bsd.c
        src/detection/gtk_qt/qt.c
        src/detection/sound/sound_bsd.c
        src/detection/swap/swap_bsd.c
        src/detection/terminalfont/terminalfont_linux.c
        src/detection/terminalshell/terminalshell_linux.c
        src/detection/terminalsize/terminalsize_linux.c
        src/detection/theme/theme_linux.c
        src/detection/tpm/tpm_bsd.c
        src/detection/uptime/uptime_bsd.c
        src/detection/users/users_linux.c
        src/detection/wallpaper/wallpaper_linux.c
        src/detection/wm/wm_linux.c
        src/detection/de/de_linux.c
        src/detection/wmtheme/wmtheme_linux.c
        src/detection/camera/camera_linux.c
        src/detection/zpool/zpool_linux.c
        src/util/platform/FFPlatform_unix.c
        src/util/binary_linux.c
    )
    if(DragonFly)
        list(APPEND LIBFASTFETCH_SRC
            src/detection/bluetooth/bluetooth_nosupport.c
            src/detection/wifi/wifi_nosupport.c
        )
    else()
        list(APPEND LIBFASTFETCH_SRC
            src/detection/bluetooth/bluetooth_bsd.c
            src/detection/wifi/wifi_bsd.c
        )
    endif()
elseif(NetBSD)
    list(APPEND LIBFASTFETCH_SRC
        src/common/dbus.c
        src/common/io/io_unix.c
        src/common/netif/netif_bsd.c
        src/common/networking/networking_linux.c
        src/common/processing_linux.c
        src/common/sysctl.c
        src/detection/battery/battery_nbsd.c
        src/detection/bios/bios_nbsd.c
        src/detection/bluetooth/bluetooth_bsd.c
        src/detection/bluetoothradio/bluetoothradio_nosupport.c
        src/detection/board/board_nbsd.c
        src/detection/bootmgr/bootmgr_bsd.c
        src/detection/brightness/brightness_nbsd.c
        src/detection/btrfs/btrfs_nosupport.c
        src/detection/chassis/chassis_nbsd.c
        src/detection/cpu/cpu_nbsd.c
        src/detection/cpucache/cpucache_shared.c
        src/detection/cpuusage/cpuusage_bsd.c
        src/detection/cursor/cursor_linux.c
        src/detection/disk/disk_bsd.c
        src/detection/dns/dns_linux.c
        src/detection/physicaldisk/physicaldisk_nosupport.c
        src/detection/physicalmemory/physicalmemory_linux.c
        src/detection/diskio/diskio_nbsd.c
        src/detection/displayserver/linux/displayserver_linux.c
        src/detection/displayserver/linux/common.c
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
        src/detection/gpu/gpu_general.c
        src/detection/gpu/gpu_pci.c
        src/detection/gtk_qt/gtk.c
        src/detection/host/host_nbsd.c
        src/detection/lm/lm_linux.c
        src/detection/icons/icons_linux.c
        src/detection/initsystem/initsystem_linux.c
        src/detection/keyboard/keyboard_nosupport.c
        src/detection/libc/libc_nosupport.c
        src/detection/loadavg/loadavg_bsd.c
        src/detection/locale/locale_linux.c
        src/detection/localip/localip_linux.c
        src/detection/gamepad/gamepad_nosupport.c
        src/detection/media/media_linux.c
        src/detection/memory/memory_nbsd.c
        src/detection/mouse/mouse_nosupport.c
        src/detection/netio/netio_bsd.c
        src/detection/opengl/opengl_linux.c
        src/detection/os/os_nbsd.c
        src/detection/packages/packages_nbsd.c
        src/detection/poweradapter/poweradapter_nosupport.c
        src/detection/processes/processes_nbsd.c
        src/detection/gtk_qt/qt.c
        src/detection/sound/sound_nbsd.c
        src/detection/swap/swap_obsd.c
        src/detection/terminalfont/terminalfont_linux.c
        src/detection/terminalshell/terminalshell_linux.c
        src/detection/terminalsize/terminalsize_linux.c
        src/detection/theme/theme_linux.c
        src/detection/tpm/tpm_nosupport.c
        src/detection/uptime/uptime_bsd.c
        src/detection/users/users_linux.c
        src/detection/wallpaper/wallpaper_linux.c
        src/detection/wifi/wifi_nbsd.c
        src/detection/wm/wm_linux.c
        src/detection/de/de_linux.c
        src/detection/wmtheme/wmtheme_linux.c
        src/detection/camera/camera_linux.c
        src/detection/zpool/zpool_linux.c
        src/util/platform/FFPlatform_unix.c
        src/util/binary_linux.c
    )
elseif(OpenBSD)
    list(APPEND LIBFASTFETCH_SRC
        src/common/dbus.c
        src/common/io/io_unix.c
        src/common/netif/netif_bsd.c
        src/common/networking/networking_linux.c
        src/common/processing_linux.c
        src/common/sysctl.c
        src/detection/battery/battery_obsd.c
        src/detection/bios/bios_windows.c
        src/detection/bluetooth/bluetooth_nosupport.c
        src/detection/bluetoothradio/bluetoothradio_nosupport.c
        src/detection/board/board_windows.c
        src/detection/bootmgr/bootmgr_bsd.c
        src/detection/brightness/brightness_obsd.c
        src/detection/btrfs/btrfs_nosupport.c
        src/detection/chassis/chassis_windows.c
        src/detection/cpu/cpu_obsd.c
        src/detection/cpucache/cpucache_shared.c
        src/detection/cpuusage/cpuusage_bsd.c
        src/detection/cursor/cursor_linux.c
        src/detection/disk/disk_bsd.c
        src/detection/dns/dns_linux.c
        src/detection/physicaldisk/physicaldisk_nosupport.c
        src/detection/physicalmemory/physicalmemory_linux.c
        src/detection/diskio/diskio_obsd.c
        src/detection/displayserver/linux/displayserver_linux.c
        src/detection/displayserver/linux/common.c
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
        src/detection/gpu/gpu_pci.c
        src/detection/gpu/gpu_general.c
        src/detection/gtk_qt/gtk.c
        src/detection/host/host_obsd.c
        src/detection/lm/lm_nosupport.c
        src/detection/icons/icons_linux.c
        src/detection/initsystem/initsystem_linux.c
        src/detection/keyboard/keyboard_nosupport.c
        src/detection/libc/libc_nosupport.c
        src/detection/loadavg/loadavg_bsd.c
        src/detection/locale/locale_linux.c
        src/detection/localip/localip_linux.c
        src/detection/gamepad/gamepad_nosupport.c
        src/detection/media/media_linux.c
        src/detection/memory/memory_obsd.c
        src/detection/mouse/mouse_nosupport.c
        src/detection/netio/netio_bsd.c
        src/detection/opengl/opengl_linux.c
        src/detection/os/os_obsd.c
        src/detection/packages/packages_obsd.c
        src/detection/poweradapter/poweradapter_nosupport.c
        src/detection/processes/processes_obsd.c
        src/detection/gtk_qt/qt.c
        src/detection/sound/sound_obsd.c
        src/detection/swap/swap_obsd.c
        src/detection/terminalfont/terminalfont_linux.c
        src/detection/terminalshell/terminalshell_linux.c
        src/detection/terminalsize/terminalsize_linux.c
        src/detection/theme/theme_linux.c
        src/detection/tpm/tpm_nosupport.c
        src/detection/uptime/uptime_bsd.c
        src/detection/users/users_obsd.c
        src/detection/wallpaper/wallpaper_linux.c
        src/detection/wifi/wifi_obsd.c
        src/detection/wm/wm_linux.c
        src/detection/de/de_linux.c
        src/detection/wmtheme/wmtheme_linux.c
        src/detection/camera/camera_linux.c
        src/detection/zpool/zpool_nosupport.c
        src/util/platform/FFPlatform_unix.c
        src/util/binary_linux.c
        src/util/smbiosHelper.c
    )
elseif(APPLE)
    list(APPEND LIBFASTFETCH_SRC
        src/common/io/io_unix.c
        src/common/netif/netif_apple.c
        src/common/networking/networking_linux.c
        src/common/processing_linux.c
        src/common/sysctl.c
        src/detection/battery/battery_apple.c
        src/detection/bios/bios_apple.c
        src/detection/bluetooth/bluetooth_apple.m
        src/detection/bluetoothradio/bluetoothradio_apple.m
        src/detection/board/board_apple.c
        src/detection/bootmgr/bootmgr_apple.c
        src/detection/brightness/brightness_apple.c
        src/detection/btrfs/btrfs_nosupport.c
        src/detection/chassis/chassis_apple.c
        src/detection/cpu/cpu_apple.c
        src/detection/cpucache/cpucache_apple.c
        src/detection/cpuusage/cpuusage_apple.c
        src/detection/cursor/cursor_apple.m
        src/detection/disk/disk_bsd.c
        src/detection/dns/dns_apple.c
        src/detection/physicaldisk/physicaldisk_apple.c
        src/detection/physicalmemory/physicalmemory_apple.m
        src/detection/diskio/diskio_apple.c
        src/detection/displayserver/displayserver_apple.c
        src/detection/font/font_apple.m
        src/detection/gpu/gpu_apple.c
        src/detection/gpu/gpu_apple.m
        src/detection/host/host_apple.c
        src/detection/host/host_mac.c
        src/detection/icons/icons_nosupport.c
        src/detection/initsystem/initsystem_linux.c
        src/detection/keyboard/keyboard_apple.c
        src/detection/lm/lm_nosupport.c
        src/detection/loadavg/loadavg_bsd.c
        src/detection/libc/libc_apple.c
        src/detection/locale/locale_linux.c
        src/detection/localip/localip_linux.c
        src/detection/gamepad/gamepad_apple.c
        src/detection/media/media_apple.m
        src/detection/memory/memory_apple.c
        src/detection/mouse/mouse_apple.c
        src/detection/netio/netio_apple.c
        src/detection/opengl/opengl_apple.c
        src/detection/os/os_apple.m
        src/detection/packages/packages_apple.c
        src/detection/packages/packages_nix.c
        src/detection/poweradapter/poweradapter_apple.c
        src/detection/processes/processes_bsd.c
        src/detection/sound/sound_apple.c
        src/detection/swap/swap_apple.c
        src/detection/terminalfont/terminalfont_apple.m
        src/detection/terminalshell/terminalshell_linux.c
        src/detection/terminalsize/terminalsize_linux.c
        src/detection/theme/theme_apple.c
        src/detection/tpm/tpm_apple.c
        src/detection/uptime/uptime_bsd.c
        src/detection/users/users_linux.c
        src/detection/wallpaper/wallpaper_apple.m
        src/detection/wifi/wifi_apple.m
        src/detection/wm/wm_apple.m
        src/detection/de/de_nosupport.c
        src/detection/wmtheme/wmtheme_apple.m
        src/detection/camera/camera_apple.m
        src/detection/zpool/zpool_nosupport.c
        src/util/apple/cf_helpers.c
        src/util/apple/osascript.m
        src/util/apple/smc_temps.c
        src/util/platform/FFPlatform_unix.c
        src/util/binary_apple.c
    )
elseif(WIN32)
    list(APPEND LIBFASTFETCH_SRC
        src/common/io/io_windows.c
        src/common/netif/netif_windows.c
        src/common/networking/networking_windows.c
        src/common/processing_windows.c
        src/detection/battery/battery_windows.c
        src/detection/bios/bios_windows.c
        src/detection/bluetooth/bluetooth_windows.c
        src/detection/bluetooth/bluetooth_windows.cpp
        src/detection/bluetoothradio/bluetoothradio_windows.c
        src/detection/board/board_windows.c
        src/detection/bootmgr/bootmgr_windows.c
        src/detection/brightness/brightness_windows.cpp
        src/detection/btrfs/btrfs_nosupport.c
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
        src/detection/host/host_mac.c
        src/detection/host/host_windows.c
        src/detection/icons/icons_windows.c
        src/detection/initsystem/initsystem_nosupport.c
        src/detection/keyboard/keyboard_windows.c
        src/detection/libc/libc_windows.cpp
        src/detection/lm/lm_nosupport.c
        src/detection/loadavg/loadavg_nosupport.c
        src/detection/locale/locale_windows.c
        src/detection/localip/localip_windows.c
        src/detection/gamepad/gamepad_windows.c
        src/detection/media/media_windows.c
        src/detection/memory/memory_windows.c
        src/detection/mouse/mouse_windows.c
        src/detection/physicalmemory/physicalmemory_linux.c
        src/detection/netio/netio_windows.c
        src/detection/opengl/opengl_windows.c
        src/detection/os/os_windows.c
        src/detection/packages/packages_windows.c
        src/detection/poweradapter/poweradapter_nosupport.c
        src/detection/processes/processes_windows.c
        src/detection/sound/sound_windows.cpp
        src/detection/swap/swap_windows.c
        src/detection/terminalfont/terminalfont_windows.c
        src/detection/terminalshell/terminalshell_windows.c
        src/detection/terminalsize/terminalsize_windows.c
        src/detection/theme/theme_windows.c
        src/detection/tpm/tpm_windows.c
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
        src/util/windows/version.c
        src/util/platform/FFPlatform_windows.c
        src/util/binary_windows.c
        src/util/debug_windows.c
    )
elseif(SunOS)
    list(APPEND LIBFASTFETCH_SRC
        src/common/dbus.c
        src/common/io/io_unix.c
        src/common/netif/netif_apple.c
        src/common/networking/networking_linux.c
        src/common/processing_linux.c
        src/detection/battery/battery_nosupport.c
        src/detection/bios/bios_windows.c
        src/detection/board/board_windows.c
        src/detection/bootmgr/bootmgr_nosupport.c
        src/detection/brightness/brightness_nosupport.c
        src/detection/btrfs/btrfs_nosupport.c
        src/detection/chassis/chassis_windows.c
        src/detection/cpu/cpu_sunos.c
        src/detection/cpucache/cpucache_shared.c
        src/detection/cpuusage/cpuusage_sunos.c
        src/detection/cursor/cursor_linux.c
        src/detection/bluetooth/bluetooth_nosupport.c
        src/detection/bluetoothradio/bluetoothradio_nosupport.c
        src/detection/disk/disk_sunos.c
        src/detection/dns/dns_linux.c
        src/detection/physicaldisk/physicaldisk_sunos.c
        src/detection/physicalmemory/physicalmemory_linux.c
        src/detection/diskio/diskio_sunos.c
        src/detection/displayserver/linux/displayserver_linux.c
        src/detection/displayserver/linux/common.c
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
        src/detection/keyboard/keyboard_nosupport.c
        src/detection/libc/libc_nosupport.c
        src/detection/lm/lm_nosupport.c
        src/detection/loadavg/loadavg_sunos.c
        src/detection/locale/locale_linux.c
        src/detection/localip/localip_linux.c
        src/detection/gamepad/gamepad_nosupport.c
        src/detection/media/media_linux.c
        src/detection/memory/memory_sunos.c
        src/detection/mouse/mouse_nosupport.c
        src/detection/netio/netio_sunos.c
        src/detection/opengl/opengl_linux.c
        src/detection/os/os_sunos.c
        src/detection/packages/packages_sunos.c
        src/detection/poweradapter/poweradapter_nosupport.c
        src/detection/processes/processes_linux.c
        src/detection/gtk_qt/qt.c
        src/detection/sound/sound_sunos.c
        src/detection/swap/swap_sunos.c
        src/detection/terminalfont/terminalfont_linux.c
        src/detection/terminalshell/terminalshell_linux.c
        src/detection/terminalsize/terminalsize_linux.c
        src/detection/theme/theme_linux.c
        src/detection/tpm/tpm_nosupport.c
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
elseif(Haiku)
    list(APPEND LIBFASTFETCH_SRC
        src/common/dbus.c
        src/common/io/io_unix.c
        src/common/netif/netif_haiku.c
        src/common/networking/networking_linux.c
        src/common/processing_linux.c
        src/detection/battery/battery_haiku.c
        src/detection/bios/bios_windows.c
        src/detection/board/board_windows.c
        src/detection/bootmgr/bootmgr_nosupport.c
        src/detection/brightness/brightness_nosupport.c
        src/detection/btrfs/btrfs_nosupport.c
        src/detection/chassis/chassis_windows.c
        src/detection/cpu/cpu_haiku.c
        src/detection/cpucache/cpucache_shared.c
        src/detection/cpuusage/cpuusage_haiku.c
        src/detection/cursor/cursor_nosupport.c
        src/detection/bluetooth/bluetooth_haiku.cpp
        src/detection/bluetoothradio/bluetoothradio_nosupport.c
        src/detection/disk/disk_haiku.cpp
        src/detection/dns/dns_linux.c
        src/detection/physicaldisk/physicaldisk_haiku.c
        src/detection/physicalmemory/physicalmemory_linux.c
        src/detection/diskio/diskio_nosupport.c
        src/detection/displayserver/displayserver_haiku.cpp
        src/detection/font/font_haiku.cpp
        src/detection/gpu/gpu_haiku.c
        src/detection/gpu/gpu_pci.c
        src/detection/gtk_qt/gtk.c
        src/detection/host/host_windows.c
        src/detection/icons/icons_nosupport.c
        src/detection/initsystem/initsystem_haiku.c
        src/detection/keyboard/keyboard_haiku.cpp
        src/detection/libc/libc_nosupport.c
        src/detection/lm/lm_nosupport.c
        src/detection/loadavg/loadavg_nosupport.c
        src/detection/locale/locale_linux.c
        src/detection/localip/localip_linux.c
        src/detection/gamepad/gamepad_haiku.cpp
        src/detection/media/media_linux.c
        src/detection/memory/memory_haiku.c
        src/detection/mouse/mouse_haiku.cpp
        src/detection/netio/netio_haiku.cpp
        src/detection/opengl/opengl_haiku.cpp
        src/detection/os/os_haiku.c
        src/detection/packages/packages_haiku.c
        src/detection/poweradapter/poweradapter_nosupport.c
        src/detection/processes/processes_haiku.c
        src/detection/gtk_qt/qt.c
        src/detection/sound/sound_haiku.cpp
        src/detection/swap/swap_haiku.c
        src/detection/terminalfont/terminalfont_linux.c
        src/detection/terminalshell/terminalshell_linux.c
        src/detection/terminalsize/terminalsize_linux.c
        src/detection/theme/theme_nosupport.c
        src/detection/tpm/tpm_nosupport.c
        src/detection/uptime/uptime_haiku.c
        src/detection/users/users_linux.c
        src/detection/wallpaper/wallpaper_nosupport.c
        src/detection/wifi/wifi_nosupport.c
        src/detection/wm/wm_nosupport.c
        src/detection/de/de_nosupport.c
        src/detection/wmtheme/wmtheme_nosupport.c
        src/detection/camera/camera_nosupport.c
        src/detection/zpool/zpool_nosupport.c
        src/util/platform/FFPlatform_unix.c
        src/util/binary_linux.c
        src/util/haiku/version.cpp
    )
elseif(GNU)
    list(APPEND LIBFASTFETCH_SRC
        src/common/dbus.c
        src/common/io/io_unix.c
        src/common/netif/netif_gnu.c
        src/common/networking/networking_linux.c
        src/common/processing_linux.c
        src/detection/battery/battery_nosupport.c
        src/detection/bios/bios_nosupport.c
        src/detection/board/board_nosupport.c
        src/detection/bootmgr/bootmgr_nosupport.c
        src/detection/brightness/brightness_nosupport.c
        src/detection/btrfs/btrfs_nosupport.c
        src/detection/chassis/chassis_nosupport.c
        src/detection/cpu/cpu_linux.c
        src/detection/cpucache/cpucache_nosupport.c
        src/detection/cpuusage/cpuusage_linux.c
        src/detection/cursor/cursor_linux.c
        src/detection/bluetooth/bluetooth_linux.c
        src/detection/bluetoothradio/bluetoothradio_linux.c
        src/detection/disk/disk_linux.c
        src/detection/dns/dns_linux.c
        src/detection/physicaldisk/physicaldisk_nosupport.c
        src/detection/physicalmemory/physicalmemory_nosupport.c
        src/detection/diskio/diskio_nosupport.c
        src/detection/displayserver/linux/displayserver_linux.c
        src/detection/displayserver/linux/common.c
        src/detection/displayserver/linux/drm.c
        src/detection/displayserver/linux/wayland/wayland.c
        src/detection/displayserver/linux/wayland/global-output.c
        src/detection/displayserver/linux/wayland/zwlr-output.c
        src/detection/displayserver/linux/wayland/wlr-output-management-unstable-v1-protocol.c
        src/detection/displayserver/linux/wmde.c
        src/detection/displayserver/linux/xcb.c
        src/detection/displayserver/linux/xlib.c
        src/detection/font/font_linux.c
        src/detection/gpu/gpu_nosupport.c
        src/detection/gpu/gpu_pci.c
        src/detection/gtk_qt/gtk.c
        src/detection/host/host_nosupport.c
        src/detection/icons/icons_linux.c
        src/detection/initsystem/initsystem_linux.c
        src/detection/keyboard/keyboard_nosupport.c
        src/detection/libc/libc_linux.c
        src/detection/lm/lm_linux.c
        src/detection/loadavg/loadavg_linux.c
        src/detection/locale/locale_linux.c
        src/detection/localip/localip_linux.c
        src/detection/gamepad/gamepad_nosupport.c
        src/detection/media/media_linux.c
        src/detection/memory/memory_linux.c
        src/detection/mouse/mouse_nosupport.c
        src/detection/netio/netio_nosupport.c
        src/detection/opengl/opengl_linux.c
        src/detection/os/os_linux.c
        src/detection/packages/packages_linux.c
        src/detection/packages/packages_nix.c
        src/detection/poweradapter/poweradapter_nosupport.c
        src/detection/processes/processes_linux.c
        src/detection/gtk_qt/qt.c
        src/detection/sound/sound_linux.c
        src/detection/swap/swap_linux.c
        src/detection/terminalfont/terminalfont_linux.c
        src/detection/terminalshell/terminalshell_linux.c
        src/detection/terminalsize/terminalsize_linux.c
        src/detection/theme/theme_linux.c
        src/detection/tpm/tpm_nosupport.c
        src/detection/uptime/uptime_linux.c
        src/detection/users/users_linux.c
        src/detection/wallpaper/wallpaper_linux.c
        src/detection/wifi/wifi_nosupport.c
        src/detection/wm/wm_linux.c
        src/detection/de/de_linux.c
        src/detection/wmtheme/wmtheme_linux.c
        src/detection/camera/camera_nosupport.c
        src/detection/zpool/zpool_nosupport.c
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
    list(APPEND LIBFASTFETCH_SRC src/util/wcwidth.c)
endif()
if(NOT WIN32)
    check_function_exists(pipe2 HAVE_PIPE2)
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

if(ANDROID)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,/vendor/lib64 -Wl,-rpath,/system/lib64")
    else()
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,/vendor/lib -Wl,-rpath,/system/lib")
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
if(MidnightBSD AND EXISTS "/usr/bin/objdump")
    execute_process(COMMAND /bin/sh -c "/usr/bin/objdump -T /lib/libc.so.* | grep 'FBSD_[0-9][0-9]*\\.[0-9][0-9]*' -o | sort -Vru | head -1"
                    OUTPUT_VARIABLE OBJDUMP_T_RESULT)
    if("${OBJDUMP_T_RESULT}" MATCHES "FBSD_([0-9]+\\.[0-9]+)")
        message(STATUS "Found FBSD ${CMAKE_MATCH_1}")
        target_compile_definitions(libfastfetch PUBLIC FF_FBSD_VERSION="${CMAKE_MATCH_1}")
    endif()
elseif(FreeBSD AND EXISTS "/usr/local/bin/objdump")
    execute_process(COMMAND /bin/sh -c "/usr/local/bin/objdump -T /lib/libc.so.* | grep 'FBSD_[0-9][0-9]*\\.[0-9][0-9]*' -o | sort -Vru | head -1"
                    OUTPUT_VARIABLE OBJDUMP_T_RESULT)
    if("${OBJDUMP_T_RESULT}" MATCHES "FBSD_([0-9]+\\.[0-9]+)")
        message(STATUS "Found FBSD ${CMAKE_MATCH_1}")
        target_compile_definitions(libfastfetch PUBLIC FF_FBSD_VERSION="${CMAKE_MATCH_1}")
    endif()
elseif(DragonFly AND EXISTS "/usr/local/bin/objdump")
    execute_process(COMMAND /bin/sh -c "/usr/local/bin/objdump -T /lib/libc.so.* | grep 'DF[0-9][0-9]*\\.[0-9][0-9]*' -o | sort -Vru | head -1"
                    OUTPUT_VARIABLE OBJDUMP_T_RESULT)
    if("${OBJDUMP_T_RESULT}" MATCHES "DF([0-9]+\\.[0-9]+)")
        message(STATUS "Found DF ${CMAKE_MATCH_1}")
        target_compile_definitions(libfastfetch PUBLIC FF_DF_VERSION="${CMAKE_MATCH_1}")
    endif()
endif()

if(LINUX)
    target_compile_definitions(libfastfetch PUBLIC _GNU_SOURCE _XOPEN_SOURCE _ATFILE_SOURCE __STDC_WANT_LIB_EXT1__ _FILE_OFFSET_BITS=64 _TIME_BITS=64) # "$<$<CONFIG:Release>:_FORTIFY_SOURCE=3>"
elseif(ANDROID)
    target_compile_definitions(libfastfetch PUBLIC _GNU_SOURCE _XOPEN_SOURCE _FILE_OFFSET_BITS=64 "$<$<CONFIG:DEBUG>:__BIONIC_FORTIFY>" "$<$<CONFIG:DEBUG>:__BIONIC_FORTIFY_RUNTIME_CHECKS_ENABLED>")
elseif(WIN32)
    target_compile_definitions(libfastfetch PUBLIC _GNU_SOURCE WIN32_LEAN_AND_MEAN _WIN32_WINNT=0x0A00 NOMINMAX UNICODE) # "$<$<CONFIG:Release>:_FORTIFY_SOURCE=3>"
elseif(APPLE)
    target_compile_definitions(libfastfetch PUBLIC _GNU_SOURCE _XOPEN_SOURCE __STDC_WANT_LIB_EXT1__ _FILE_OFFSET_BITS=64 _DARWIN_C_SOURCE)
elseif(OpenBSD)
    target_compile_definitions(libfastfetch PUBLIC _XOPEN_SOURCE=700 _FILE_OFFSET_BITS=64 _BSD_SOURCE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,/usr/X11R6/lib") # detect x11 lib path automatically
elseif(DragonFly)
    target_compile_definitions(libfastfetch PUBLIC __FreeBSD__)
elseif(SunOS)
    target_compile_definitions(libfastfetch PUBLIC _GNU_SOURCE _XOPEN_SOURCE __STDC_WANT_LIB_EXT1__ _FILE_OFFSET_BITS=64 __EXTENSIONS__ _POSIX_C_SOURCE)
elseif(NetBSD)
    target_compile_definitions(libfastfetch PUBLIC _GNU_SOURCE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-char-subscripts")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,/usr/X11R7/lib -Wl,-rpath,/usr/pkg/lib") # ditto
elseif(Haiku)
    target_compile_definitions(libfastfetch PUBLIC _GNU_SOURCE)
elseif(GNU)
    target_compile_definitions(libfastfetch PUBLIC _GNU_SOURCE)
    # On Hurd PATH_MAX is not defined. Set an arbitrary limit as workaround.
    target_compile_definitions(libfastfetch PUBLIC PATH_MAX=4096)
endif()

if(FreeBSD OR OpenBSD OR NetBSD)
    include(CheckStructHasMember)
    set(CMAKE_REQUIRED_DEFINITIONS "-D_IFI_OQDROPS=1")
    CHECK_STRUCT_HAS_MEMBER("struct if_data" ifi_oqdrops net/if.h HAVE_IFI_OQDROPS LANGUAGE C)
    if(HAVE_IFI_OQDROPS)
    target_compile_definitions(libfastfetch PUBLIC _IFI_OQDROPS FF_HAVE_IFI_OQDROPS)
    endif()
    unset(CMAKE_REQUIRED_DEFINITIONS)
endif()

if(HAVE_WCWIDTH)
    target_compile_definitions(libfastfetch PUBLIC FF_HAVE_WCWIDTH)
endif()

if(HAVE_PIPE2)
    target_compile_definitions(libfastfetch PUBLIC FF_HAVE_PIPE2)
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
if(NOT "${CUSTOM_LSB_RELEASE_PATH}" STREQUAL "")
    message(STATUS "Custom file path of lsb_release: ${CUSTOM_LSB_RELEASE_PATH}")
    target_compile_definitions(libfastfetch PRIVATE FF_CUSTOM_LSB_RELEASE_PATH=${CUSTOM_LSB_RELEASE_PATH})
endif()

if(NOT BINARY_LINK_TYPE STREQUAL "dlopen")
    message(STATUS "Enabling custom link type: ${BINARY_LINK_TYPE}")
    target_compile_definitions(libfastfetch PRIVATE FF_DISABLE_DLOPEN=1)
    if(NOT WIN32)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--copy-dt-needed-entries")
    endif()
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
ff_lib_enable(XRANDR
    "xrandr"
    "XRandr"
)
ff_lib_enable(DRM
    "libdrm"
    "Libdrm"
)
ff_lib_enable(DRM_AMDGPU
    "libdrm_amdgpu"
    "Libdrm_Amdgpu"
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
if(NOT OpenBSD AND NOT NetBSD)
    ff_lib_enable(GLX
        "glx"
        "GLX"
    )
else()
    ff_lib_enable(GLX
        "gl"
        "GL"
    )
endif()
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
ff_lib_enable(PCIACCESS
    "pciaccess"
    "pciaccess"
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
    target_compile_definitions(libfastfetch PRIVATE FF_HAVE_THREADS=1)
    if(CMAKE_USE_PTHREADS_INIT) #Threads::Threads is not set for WIN32
        target_link_libraries(libfastfetch PRIVATE Threads::Threads)
    endif()
endif()

if(ENABLE_EMBEDDED_PCIIDS)
    target_compile_definitions(libfastfetch PRIVATE FF_HAVE_EMBEDDED_PCIIDS=1)
endif()
if(ENABLE_EMBEDDED_AMDGPUIDS)
    target_compile_definitions(libfastfetch PRIVATE FF_HAVE_EMBEDDED_AMDGPUIDS=1)
endif()

if(LINUX)
    target_link_libraries(libfastfetch
        PRIVATE "m"
    )

    if(ENABLE_DIRECTX_HEADERS)
        if(NOT BINARY_LINK_TYPE STREQUAL "dlopen")
            target_link_libraries(libfastfetch
                PRIVATE "/usr/lib/wsl/lib/libdxcore.so"
            )
        endif()
    endif()
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
    )
elseif(WIN32)
    target_link_libraries(libfastfetch
        PRIVATE "dwmapi"
        PRIVATE "gdi32"
        PRIVATE "iphlpapi"
        PRIVATE "ole32"
        PRIVATE "oleaut32"
        PRIVATE "ws2_32"
        PRIVATE "ntdll"
        PRIVATE "version"
        PRIVATE "hid"
        PRIVATE "wtsapi32"
        PRIVATE "imagehlp"
        PRIVATE "cfgmgr32"
        PRIVATE "winbrand"
        PRIVATE "propsys"
        PRIVATE "secur32"
    )
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
        # WoA only works on Windows 10 or higher
        target_link_libraries(libfastfetch
            PRIVATE "mincore"
        )
    endif()
elseif(FreeBSD)
    target_link_libraries(libfastfetch
        PRIVATE "m"
        PRIVATE "usbhid"
        PRIVATE "bluetooth"
    )
    if(NOT DragonFly)
        target_link_libraries(libfastfetch
            PRIVATE "geom"
        )
    else()
        target_link_libraries(libfastfetch
            PRIVATE "devstat"
        )
    endif()
elseif(OpenBSD)
    target_link_libraries(libfastfetch
        PRIVATE "m"
        PRIVATE "kvm"
        PRIVATE "sndio"
    )
elseif(NetBSD)
    target_link_libraries(libfastfetch
        PRIVATE "bluetooth"
        PRIVATE "m"
        PRIVATE "prop"
    )
elseif(SunOS)
    target_link_libraries(libfastfetch
        PRIVATE "m"
        PRIVATE "socket"
        PRIVATE "kstat"
        PRIVATE "proc"
        PRIVATE "zfs"
        PRIVATE "nvpair"
        PRIVATE "devinfo"
    )
elseif(GNU)
    target_link_libraries(libfastfetch
        PRIVATE "m"
    )
elseif(ANDROID)
    target_link_libraries(libfastfetch
        PRIVATE "m"
    )
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
elseif(Haiku)
    target_link_libraries(libfastfetch
        PRIVATE "network"
        PRIVATE "bnetapi"
        PRIVATE "media"
        PRIVATE "device"
        PRIVATE "bluetooth"
        PRIVATE "GL"
        PRIVATE "be"
        PRIVATE "gnu"
    )
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
if(LINUX OR FreeBSD OR OpenBSD OR NetBSD)
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
    CHECK_INCLUDE_FILE("glob.h" HAVE_GLOB)
    if(HAVE_GLOB)
        target_compile_definitions(libfastfetch PRIVATE FF_HAVE_GLOB=1)
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

# Prevent fastfetch from linking to libstdc++
set(CMAKE_CXX_IMPLICIT_LINK_LIBRARIES "")
set(CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES "")
set_target_properties(fastfetch PROPERTIES LINKER_LANGUAGE C)

if(WIN32)
    target_sources(fastfetch
        PRIVATE src/util/windows/version.rc
    )
elseif(APPLE)
    target_link_options(fastfetch
        PRIVATE LINKER:-sectcreate,__TEXT,__info_plist,Info.plist
    )
endif()

if(BINARY_LINK_TYPE STREQUAL "static")
    target_link_options(fastfetch PRIVATE "-static")
endif()

# Apply all above parameters to flashfetch if it is built
if (BUILD_FLASHFETCH)
    add_executable(flashfetch
        src/flashfetch.c
    )
    target_compile_definitions(flashfetch
        PRIVATE FASTFETCH_TARGET_BINARY_NAME=flashfetch
    )
    target_link_libraries(flashfetch
        PRIVATE libfastfetch
    )

    set_target_properties(flashfetch PROPERTIES LINKER_LANGUAGE C)

    if(WIN32)
        target_sources(flashfetch
            PRIVATE src/util/windows/version.rc
        )
    elseif(APPLE)
        target_link_options(flashfetch
            PRIVATE LINKER:-sectcreate,__TEXT,__info_plist,Info.plist
        )
    endif()

    if(BINARY_LINK_TYPE STREQUAL "static")
        target_link_options(flashfetch PRIVATE "-static")
    endif()
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

    add_executable(fastfetch-test-color
        tests/color.c
    )
    target_link_libraries(fastfetch-test-color
        PRIVATE libfastfetch
    )

    add_executable(fastfetch-test-duration
        tests/duration.c
    )
    target_link_libraries(fastfetch-test-duration
        PRIVATE libfastfetch
    )

    enable_testing()
    add_test(NAME test-strbuf COMMAND fastfetch-test-strbuf)
    add_test(NAME test-list COMMAND fastfetch-test-list)
    add_test(NAME test-format COMMAND fastfetch-test-format)
    add_test(NAME test-color COMMAND fastfetch-test-color)
    add_test(NAME test-duration COMMAND fastfetch-test-duration)
endif()

##################
# install target #
##################

include(GNUInstallDirs)

install(
    TARGETS fastfetch
    DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

if (TARGET flashfetch)
    install(
        TARGETS flashfetch
        DESTINATION "${CMAKE_INSTALL_BINDIR}"
    )
endif()

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
    string(TOLOWER "${CMAKE_PROJECT_NAME}-macos-${CMAKE_SYSTEM_PROCESSOR}" CPACK_PACKAGE_FILE_NAME) # use macos instead of darwin
elseif(IS_MUSL)
    string(TOLOWER "${CMAKE_PROJECT_NAME}-musl-${CMAKE_SYSTEM_PROCESSOR}" CPACK_PACKAGE_FILE_NAME)
else()
    string(TOLOWER "${CMAKE_PROJECT_NAME}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}" CPACK_PACKAGE_FILE_NAME)
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

    if(NOT IS_MUSL)
        find_program(HAVE_RPMBUILD "rpmbuild")
        if(HAVE_RPMBUILD)
            set(CPACK_GENERATOR "${CPACK_GENERATOR};RPM")

            set(CPACK_RPM_PACKAGE_LICENSE "MIT")
        endif()
    endif()
elseif(FreeBSD)
    set(CPACK_FREEBSD_PACKAGE_LICENSE "MIT")
    set(CPACK_GENERATOR "${CPACK_GENERATOR};FREEBSD")
endif()

set(CPACK_SET_DESTDIR ON)

set(CPACK_PACKAGE_CONTACT "Carter Li <zhangsongcui@live.cn>")
set(CPACK_PACKAGE_DESCRIPTION "\
fastfetch is a neofetch-like tool for fetching system information and displaying them in a pretty way. \
It is written mostly in C to achieve much better performance.\
")

include(CPack)

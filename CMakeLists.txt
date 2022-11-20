cmake_minimum_required(VERSION 3.12.0) # target_link_libraries with OBJECT libs & project homepage url

project(fastfetch
    VERSION 1.7.5
    LANGUAGES C
    DESCRIPTION "Fast system information tool"
    HOMEPAGE_URL "https://github.com/LinusDierheimer/fastfetch"
)

set(PROJECT_LICENSE "MIT license")

###################
# Target Platform #
###################

if("${CMAKE_SYSTEM_NAME}" MATCHES ".*[Ll]inux.*")
    set(LINUX TRUE CACHE BOOL "..." FORCE) # LINUX means GNU/Linux, not just the kernel
elseif("${CMAKE_SYSTEM_NAME}" MATCHES ".*[Bb][Ss][Dd].*")
    set(BSD TRUE CACHE BOOL "..." FORCE)
elseif(NOT APPLE AND NOT ANDROID AND NOT WIN32)
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

find_package(PkgConfig REQUIRED)

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
cmake_dependent_option(ENABLE_SQLITE3 "Enable sqlite3" ON "LINUX OR BSD" OFF)
cmake_dependent_option(ENABLE_RPM "Enable rpm" ON "LINUX" OFF)
cmake_dependent_option(ENABLE_IMAGEMAGICK7 "Enable imagemagick 7" ON "LINUX OR BSD" OFF)
cmake_dependent_option(ENABLE_IMAGEMAGICK6 "Enable imagemagick 6" ON "LINUX OR BSD" OFF)
cmake_dependent_option(ENABLE_CHAFA "Enable chafa" ON "ENABLE_IMAGEMAGICK6 OR ENABLE_IMAGEMAGICK7" OFF)
cmake_dependent_option(ENABLE_ZLIB "Enable zlib" ON "ENABLE_IMAGEMAGICK6 OR ENABLE_IMAGEMAGICK7" OFF)
cmake_dependent_option(ENABLE_EGL "Enable egl" ON "LINUX OR BSD" OFF)
cmake_dependent_option(ENABLE_GLX "Enable glx" ON "LINUX OR BSD" OFF)
cmake_dependent_option(ENABLE_OSMESA "Enable osmesa" ON "LINUX OR BSD" OFF)
cmake_dependent_option(ENABLE_OPENCL "Enable opencl" ON "LINUX OR BSD OR WIN32" OFF)
cmake_dependent_option(ENABLE_LIBCJSON "Enable libcjson" ON "LINUX OR WIN32" OFF)
cmake_dependent_option(ENABLE_FREETYPE "Enable freetype" ON "ANDROID" OFF)
cmake_dependent_option(ENABLE_THREADS "Enable multithreading" ON "Threads_FOUND AND NOT ANDROID" OFF)
cmake_dependent_option(USE_WIN_FAST_PPID_DETECTION "Use internal NTAPI instead of querying WMI to get PPID" ON "WIN32" OFF)

option(BUILD_TESTS "Build tests" OFF) # Also create test executables
option(SET_TWEAK "Add tweak to project version" ON) # This is set to off by github actions for release builds

####################
# Compiler options #
####################

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
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
set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} ${FASTFETCH_FLAGS_DEBUG} -rdynamic")

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
    src/util/FFstrbuf.c
    src/util/FFlist.c
    src/util/FFvaluestore.c
    src/common/init.c
    src/common/io.c
    src/common/printing.c
    src/common/caching.c
    src/common/properties.c
    src/common/font.c
    src/common/format.c
    src/common/parsing.c
    src/common/settings.c
    src/common/library.c
    src/common/bar.c
    src/logo/logo.c
    src/logo/builtin.c
    src/logo/image/image.c
    src/logo/image/im7.c
    src/logo/image/im6.c
    src/detection/vulkan.c
    src/detection/datetime.c
    src/detection/title.c
    src/detection/host/host.c
    src/detection/os/os.c
    src/detection/cpu/cpu.c
    src/detection/cpuUsage/cpuUsage.c
    src/detection/gpu/gpu.c
    src/detection/memory/memory.c
    src/detection/swap/swap.c
    src/detection/font/font.c
    src/detection/displayserver/displayserver.c
    src/detection/terminalfont/terminalfont.c
    src/detection/terminalshell/terminalshell.c
    src/detection/media/media.c
    src/detection/packages/packages.c
    src/detection/disk/disk.c
    src/modules/break.c
    src/modules/custom.c
    src/modules/title.c
    src/modules/separator.c
    src/modules/os.c
    src/modules/host.c
    src/modules/bios.c
    src/modules/board.c
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
    src/modules/poweradapter.c
    src/modules/locale.c
    src/modules/vulkan.c
    src/modules/localip.c
    src/modules/publicip.c
    src/modules/weather.c
    src/modules/player.c
    src/modules/song.c
    src/modules/datetime.c
    src/modules/date.c
    src/modules/time.c
    src/modules/colors.c
    src/modules/opengl.c
    src/modules/opencl.c
    src/modules/users.c
)

if(LINUX OR APPLE OR ANDROID OR BSD)
    list(APPEND LIBFASTFETCH_SRC
        src/common/processing_linux.c
        src/common/networking_linux.c
        src/detection/users/users_linux.c
        src/detection/terminalshell/terminalshell_linux.c
        src/detection/localip/localip_linux.c
    )
endif()

if(BSD OR APPLE)
    list(APPEND LIBFASTFETCH_SRC
        src/common/sysctl.c
        src/detection/disk/disk_bsd.c
        src/detection/uptime/uptime_bsd.c
        src/detection/processes/processes_bsd.c
    )
endif()

if(LINUX OR ANDROID)
    list(APPEND LIBFASTFETCH_SRC
        src/detection/cpu/cpu_linux.c
        src/detection/cpuUsage/cpuUsage_linux.c
        src/detection/disk/disk_linux.c
        src/detection/memory/memory_linux.c
        src/detection/processes/processes_linux.c
        src/detection/swap/swap_linux.c
        src/detection/uptime/uptime_linux.c
    )
endif()

if(LINUX OR ANDROID OR BSD)
    list(APPEND LIBFASTFETCH_SRC
        src/detection/temps/temps_linux.c
        src/detection/opengl/opengl_linux.c
        src/detection/packages/packages_linux.c

        src/detection/poweradapter/poweradapter_nosupport.c
    )
endif()

if(LINUX OR BSD)
    list(APPEND LIBFASTFETCH_SRC
        src/detection/os/os_linux.c
        src/detection/gpu/gpu_linux.c
        src/detection/displayserver/linux/displayserver_linux.c
        src/detection/displayserver/linux/wayland.c
        src/detection/displayserver/linux/xcb.c
        src/detection/displayserver/linux/xlib.c
        src/detection/displayserver/linux/wmde.c
        src/detection/terminalfont/terminalfont_linux.c
        src/detection/media/media_linux.c
        src/detection/wmtheme/wmtheme_linux.c
        src/detection/font/font_linux.c
        src/detection/qt.c
        src/detection/gtk.c
    )
endif()

if(LINUX)
    list(APPEND LIBFASTFETCH_SRC
        src/detection/host/host_linux.c
        src/detection/bios/bios_linux.c
        src/detection/board/board_linux.c
        src/detection/battery/battery_linux.c
    )
endif()

if(WIN32)
    list(APPEND LIBFASTFETCH_SRC
        src/common/processing_windows.c
        src/common/networking_windows.c
        src/detection/host/host_windows.cpp
        src/detection/bios/bios_windows.cpp
        src/detection/board/board_windows.cpp
        src/detection/cpu/cpu_windows.cpp
        src/detection/gpu/gpu_windows.cpp
        src/detection/battery/battery_windows.cpp
        src/detection/displayserver/displayserver_windows.c
        src/detection/wmtheme/wmtheme_windows.c
        src/detection/opengl/opengl_windows.c
        src/detection/users/users_windows.cpp
        src/detection/os/os_windows.cpp
        src/detection/processes/processes_windows.cpp
        src/detection/disk/disk_windows.c
        src/detection/cpuUsage/cpuUsage_windows.c
        src/detection/cpuUsage/cpuUsage_nowait_windows.cpp
        src/detection/memory/memory_windows.cpp
        src/detection/swap/swap_windows.cpp
        src/detection/font/font_windows.cpp
        src/detection/terminalfont/terminalfont_windows.c
        src/detection/localip/localip_windows.c
        src/detection/uptime/uptime_windows.c
        src/detection/packages/packages_windows.c
        src/detection/terminalshell/terminalshell_windows.cpp
        src/util/windows/wmi.cpp
        src/util/windows/getline.c
        src/util/windows/pwd.c
        src/util/windows/utsname.c

        src/detection/poweradapter/poweradapter_nosupport.c
        src/detection/media/media_nosupport.c
    )
endif()

if(APPLE)
    list(APPEND LIBFASTFETCH_SRC
        src/detection/cpuUsage/cpuUsage_apple.c
        src/util/apple/cf_helpers.c
        src/util/apple/osascript.m
        src/detection/host/host_apple.c
        src/detection/os/os_apple.m
        src/detection/cpu/cpu_apple.c
        src/detection/gpu/gpu_apple.c
        src/detection/battery/battery_apple.c
        src/detection/poweradapter/poweradapter_apple.c
        src/detection/memory/memory_apple.c
        src/detection/swap/swap_apple.c
        src/detection/displayserver/displayserver_apple.c
        src/detection/terminalfont/terminalfont_apple.m
        src/detection/media/media_apple.m
        src/detection/disk/disk_apple.m
        src/detection/wmtheme/wmtheme_apple.m
        src/detection/temps/temps_apple.c
        src/detection/font/font_apple.m
        src/detection/opengl/opengl_apple.c
        src/detection/packages/packages_apple.c

        src/detection/bios/bios_nosupport.c
        src/detection/board/board_nosupport.c
    )
endif()

if(BSD)
    list(APPEND LIBFASTFETCH_SRC
        src/detection/cpu/cpu_bsd.c
        src/detection/cpuUsage/cpuUsage_bsd.c
        src/detection/memory/memory_bsd.c
        src/detection/swap/swap_bsd.c
        src/detection/host/host_bsd.c

        src/detection/battery/battery_nosupport.c
        src/detection/bios/bios_nosupport.c
        src/detection/board/board_nosupport.c
    )
endif()

if(ANDROID)
    list(APPEND LIBFASTFETCH_SRC
        src/detection/host/host_android.c
        src/detection/os/os_android.c
        src/detection/terminalfont/terminalfont_android.c

        src/detection/bios/bios_nosupport.c
        src/detection/board/board_nosupport.c
        src/detection/displayserver/displayserver_nosupport.c
        src/detection/battery/battery_nosupport.c
        src/detection/gpu/gpu_nosupport.c
        src/detection/font/font_nosupport.c
        src/detection/media/media_nosupport.c
        src/detection/wmtheme/wmtheme_nosupport.c
    )
endif()

add_library(libfastfetch OBJECT
    ${LIBFASTFETCH_SRC}
)

target_compile_definitions(libfastfetch PUBLIC _GNU_SOURCE)

CHECK_INCLUDE_FILE("sys/sysinfo.h" HAVE_SYSINFO_H)
if(HAVE_SYSINFO_H)
    # needs to be public, because changes fastfech.h ABI
    target_compile_definitions(libfastfetch PUBLIC FF_HAVE_SYSINFO_H)
endif()

CHECK_INCLUDE_FILE("utmpx.h" HAVE_UTMPX_H)
if(HAVE_UTMPX_H)
    target_compile_definitions(libfastfetch PRIVATE FF_HAVE_UTMPX_H)
endif()

function(ff_lib_enable VARNAME)
    if(NOT ENABLE_${VARNAME})
        return()
    endif()

    pkg_search_module(${VARNAME} ${ARGN})
    if(NOT ${VARNAME}_FOUND)
        message(WARNING "Package ${ARGV1} not found, building without support.")
        return()
    endif()

    target_compile_definitions(libfastfetch PRIVATE FF_HAVE_${VARNAME}=1)
    target_include_directories(libfastfetch PRIVATE ${${VARNAME}_INCLUDE_DIRS})
    target_link_directories(libfastfetch PRIVATE ${${VARNAME}_LIBRARY_DIRS})

    foreach(FLAG ${${VARNAME}_CFLAGS_OTHER})
        if(FLAG MATCHES "-D.*")
            string(SUBSTRING ${FLAG} 2 -1 FLAG)
            target_compile_definitions(libfastfetch PRIVATE ${FLAG})
        endif()
    endforeach()
endfunction()

ff_lib_enable(LIBPCI libpci)
ff_lib_enable(VULKAN vulkan)
ff_lib_enable(WAYLAND wayland-client)
ff_lib_enable(XCB_RANDR xcb-randr)
ff_lib_enable(XCB xcb)
ff_lib_enable(XRANDR xrandr)
ff_lib_enable(X11 x11)
ff_lib_enable(GIO gio-2.0)
ff_lib_enable(DCONF dconf)
ff_lib_enable(DBUS dbus-1)
ff_lib_enable(XFCONF libxfconf-0)
ff_lib_enable(SQLITE3 sqlite3)
ff_lib_enable(RPM rpm)
ff_lib_enable(IMAGEMAGICK7 MagickCore-7.Q16HDRI MagickCore-7.Q16 MagickCore-7 /usr/lib/imagemagick7/pkgconfig/MagickCore-7.Q16HDRI.pc /usr/lib/imagemagick7/pkgconfig/MagickCore-7.Q16.pc /usr/lib/imagemagick7/pkgconfig/MagickCore-7.pc)
ff_lib_enable(IMAGEMAGICK6 MagickCore-6.Q16HDRI MagickCore-6.Q16 MagickCore-6 /usr/lib/imagemagick6/pkgconfig/MagickCore-6.Q16HDRI.pc /usr/lib/imagemagick6/pkgconfig/MagickCore-6.Q16.pc /usr/lib/imagemagick6/pkgconfig/MagickCore-6.pc)
ff_lib_enable(ZLIB zlib)
ff_lib_enable(CHAFA chafa>=1.10)
ff_lib_enable(EGL egl)
ff_lib_enable(GLX glx)
ff_lib_enable(OSMESA osmesa)
ff_lib_enable(OPENCL OpenCL)
ff_lib_enable(LIBCJSON libcjson)
ff_lib_enable(FREETYPE freetype2)

if(ENABLE_THREADS)
    target_compile_definitions(libfastfetch PRIVATE FF_HAVE_THREADS)
    if(CMAKE_USE_PTHREADS_INIT) #Threads::Threads is not set for WIN32
        target_link_libraries(libfastfetch PRIVATE Threads::Threads)
    endif()
endif()

if(APPLE)
    target_link_libraries(libfastfetch
        PRIVATE "-framework CoreFoundation"
        PRIVATE "-framework IOKit"
        PRIVATE "-framework OpenGL"
        PRIVATE "-framework OpenCL"
        PRIVATE "-framework Cocoa"
        PRIVATE "-weak_framework MediaRemote -F /System/Library/PrivateFrameworks"
    )
elseif(WIN32)
    target_link_libraries(libfastfetch
        PRIVATE "dwmapi"
        PRIVATE "gdi32"
        PRIVATE "iphlpapi"
        PRIVATE "ole32"
        PRIVATE "oleaut32"
        PRIVATE "opengl32"
        PRIVATE "wbemuuid"
        PRIVATE "ws2_32"
        PRIVATE "ntdll"
        PRIVATE "version"
    )
    if(USE_WIN_FAST_PPID_DETECTION)
        target_compile_definitions(libfastfetch PRIVATE FF_USE_WIN_FAST_PPID_DETECTION)
    endif()
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
target_link_libraries(fastfetch
    PRIVATE libfastfetch
)

add_executable(flashfetch
    src/flashfetch.c
)
target_link_libraries(flashfetch
    PRIVATE libfastfetch
)

if(WIN32)
    if(PROJECT_VERSION_TWEAK)
        string(REGEX MATCH "[0-9]+" PROJECT_VERSION_TWEAK_NUM "${PROJECT_VERSION_TWEAK}")
    else()
        set(PROJECT_VERSION_TWEAK_NUM "0")
    endif()

    set(TARGET_NAME fastfetch)
    configure_file(src/util/windows/version.rc.in version.fastfetch.rc)
    target_sources(fastfetch
        PRIVATE version.fastfetch.rc
    )

    set(TARGET_NAME flashfetch)
    configure_file(src/util/windows/version.rc.in version.flashfetch.rc)
    target_sources(flashfetch
        PRIVATE version.flashfetch.rc
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

cmake_minimum_required(VERSION 3.12.0) # target_link_libraries with OBJECT libs & project homepage url

project(fastfetch
    VERSION 1.7.2
    LANGUAGES C
    DESCRIPTION "Fast system information tool"
    HOMEPAGE_URL "https://github.com/LinusDierheimer/fastfetch"
)

###################
# Target Platform #
###################

if(APPLE OR ANDROID)
    set(LINUX FALSE CACHE BOOL "..." FORCE) # Linux true means we have full linux userland, not just the kernel
elseif(UNIX)
    set(LINUX TRUE CACHE BOOL "..." FORCE)
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

#####################
# Configure options #
#####################

include(CMakeDependentOption)

cmake_dependent_option(ENABLE_LIBPCI "Enable libpci" ON "LINUX" OFF)
cmake_dependent_option(ENABLE_VULKAN "Enable vulkan" ON "LINUX OR APPLE" OFF)
cmake_dependent_option(ENABLE_WAYLAND "Enable wayland-client" ON "LINUX" OFF)
cmake_dependent_option(ENABLE_XCB_RANDR "Enable xcb-randr" ON "LINUX" OFF)
cmake_dependent_option(ENABLE_XCB "Enable xcb" ON "LINUX" OFF)
cmake_dependent_option(ENABLE_XRANDR "Enable xrandr" ON "LINUX" OFF)
cmake_dependent_option(ENABLE_X11 "Enable x11" ON "LINUX" OFF)
cmake_dependent_option(ENABLE_GIO "Enable gio-2.0" ON "LINUX" OFF)
cmake_dependent_option(ENABLE_DCONF "Enable dconf" ON "LINUX" OFF)
cmake_dependent_option(ENABLE_DBUS "Enable dbus-1" ON "LINUX" OFF)
cmake_dependent_option(ENABLE_XFCONF "Enable libxfconf-0" ON "LINUX" OFF)
cmake_dependent_option(ENABLE_SQLITE3 "Enable sqlite3" ON "LINUX" OFF)
cmake_dependent_option(ENABLE_RPM "Enable rpm" ON "LINUX" OFF)
cmake_dependent_option(ENABLE_IMAGEMAGICK7 "Enable imagemagick 7" ON "LINUX" OFF)
cmake_dependent_option(ENABLE_IMAGEMAGICK6 "Enable imagemagick 6" ON "LINUX" OFF)
cmake_dependent_option(ENABLE_CHAFA "Enable chafa" ON "ENABLE_IMAGEMAGICK6 OR ENABLE_IMAGEMAGICK7" OFF)
cmake_dependent_option(ENABLE_ZLIB "Enable zlib" ON "ENABLE_IMAGEMAGICK6 OR ENABLE_IMAGEMAGICK7" OFF)
cmake_dependent_option(ENABLE_EGL "Enable egl" ON "LINUX" OFF)
cmake_dependent_option(ENABLE_GLX "Enable glx" ON "LINUX" OFF)
cmake_dependent_option(ENABLE_OSMESA "Enable osmesa" ON "LINUX" OFF)
cmake_dependent_option(ENABLE_OPENCL "Enable opencl" ON "LINUX" OFF)

option(BUILD_TESTS "Build tests" OFF) # Also create test executables
option(SET_TWEAK "Add tweak to project version" ON) # This is set to off by github actions for release builds

#############################
# Compile time dependencies #
#############################

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_package(PkgConfig REQUIRED)

include(CheckIncludeFile)

####################
# Compiler options #
####################

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wconversion")
if(APPLE AND DEFINED ENV{HOMEBREW_PREFIX})
    # Used for dlopen finding dylibs installed by homebrew, reversed for future usage
    # `/opt/homebrew/lib` is not on dlopen search path by default
    set(CMAKE_EXE_LINKER_FLAGS "-Wl,-rpath,$ENV{HOMEBREW_PREFIX}/lib")
endif()

include(CheckIPOSupported)
check_ipo_supported(RESULT IPO_SUPPORTED)
if(IPO_SUPPORTED)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
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
    string(REPLACE "\n" "\\n" TEMP "${TEMP}")
    string(REPLACE "\"" "\\\"" TEMP "${TEMP}")
    string(REPLACE "$\\" "" TEMP "${TEMP}")
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
    src/detection/qt.c
    src/detection/gtk.c
    src/detection/terminalShell.c
    src/detection/vulkan.c
    src/detection/media.c
    src/detection/datetime.c
    src/detection/temps.c
    src/detection/title.c
    src/detection/host/host.c
    src/detection/os/os.c
    src/detection/cpu/cpu.c
    src/detection/gpu/gpu.c
    src/detection/memory/memory.c
    src/detection/displayserver/displayserver.c
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
    src/modules/vulkan.c
    src/modules/localip.c
    src/modules/publicip.c
    src/modules/player.c
    src/modules/song.c
    src/modules/datetime.c
    src/modules/date.c
    src/modules/time.c
    src/modules/colors.c
    src/modules/opengl.c
    src/modules/opencl.c
)

if(APPLE)
    list(APPEND LIBFASTFETCH_SRC
        src/detection/host/host_apple.c
        src/detection/os/os_apple.c
        src/detection/cpu/cpu_apple.c
        src/detection/gpu/gpu_apple.c
        src/detection/memory/memory_apple.c
        src/detection/displayserver/displayserver_apple.c
    )
elseif(ANDROID)
    list(APPEND LIBFASTFETCH_SRC
        src/detection/host/host_android.c
        src/detection/os/os_android.c
        src/detection/cpu/cpu_linux.c
        src/detection/gpu/gpu_android.c
        src/detection/memory/memory_linux.c
        src/detection/displayserver/displayserver_android.c
    )
else()
    list(APPEND LIBFASTFETCH_SRC
        src/detection/host/host_linux.c
        src/detection/os/os_linux.c
        src/detection/cpu/cpu_linux.c
        src/detection/gpu/gpu_linux.c
        src/detection/memory/memory_linux.c
        src/detection/displayserver/linux/displayserver_linux.c
        src/detection/displayserver/linux/wayland.c
        src/detection/displayserver/linux/xcb.c
        src/detection/displayserver/linux/xlib.c
        src/detection/displayserver/linux/wmde.c
    )
endif()

add_library(libfastfetch OBJECT
    ${LIBFASTFETCH_SRC}
)

CHECK_INCLUDE_FILE("sys/sysinfo.h" HAVE_SYSINFO_H)
if(HAVE_SYSINFO_H)
    # needs to be public, because changes fastfech.h ABI
    target_compile_definitions(libfastfetch PUBLIC FF_HAVE_SYSINFO_H)
endif()

function(ff_lib_enable VARNAME)
    if(NOT ENABLE_${VARNAME})
        return()
    endif()

    foreach(LIBNAME ${ARGN})
        pkg_check_modules(${VARNAME} ${LIBNAME})
        if(${VARNAME}_FOUND)
            target_compile_definitions(libfastfetch PRIVATE FF_HAVE_${VARNAME}=1)
            target_include_directories(libfastfetch PRIVATE ${${VARNAME}_INCLUDE_DIRS})
            target_link_directories(libfastfetch PRIVATE ${${VARNAME}_LIBRARY_DIRS})
            return()
        endif()
    endforeach()

    message(WARNING "Package ${ARGV1} not found, building without support.")
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
ff_lib_enable(IMAGEMAGICK7 MagickCore-7.Q16HDRI MagickCore-7.Q16 /usr/lib/imagemagick7/pkgconfig/MagickCore-7.Q16HDRI.pc /usr/lib/imagemagick7/pkgconfig/MagickCore-7.Q16.pc)
ff_lib_enable(IMAGEMAGICK6 MagickCore-6.Q16HDRI MagickCore-6.Q16 /usr/lib/imagemagick6/pkgconfig/MagickCore-6.Q16HDRI.pc /usr/lib/imagemagick6/pkgconfig/MagickCore-6.Q16.pc)
ff_lib_enable(ZLIB zlib)
ff_lib_enable(CHAFA chafa>=1.10)
ff_lib_enable(EGL egl)
ff_lib_enable(GLX glx)
ff_lib_enable(OSMESA osmesa)
ff_lib_enable(OPENCL OpenCL)

if(APPLE)
    target_link_libraries(libfastfetch
        PRIVATE "-framework CoreFoundation"
        PRIVATE "-framework IOKit"
        PRIVATE "-framework CoreGraphics"
        PRIVATE "-framework OpenGL"
        PRIVATE "-framework OpenCL"
    )
endif()

target_include_directories(libfastfetch
    PUBLIC ${PROJECT_BINARY_DIR}
    PUBLIC ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(libfastfetch
    PRIVATE ${CMAKE_DL_LIBS}
    PRIVATE Threads::Threads
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

    enable_testing()
    add_test(NAME test-strbuf COMMAND fastfetch-test-strbuf)
endif()

##################
# install target #
##################

#https://cmake.org/cmake/help/latest/variable/CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT.html
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${TARGET_DIR_USR}" CACHE PATH "..." FORCE)
endif()

if(NOT CMAKE_INSTALL_SYSCONFDIR)
    set(CMAKE_INSTALL_SYSCONFDIR "${TARGET_DIR_ROOT}/etc" CACHE PATH "..." FORCE)
endif()

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
elseif(APPLE)
    set(CPACK_GENERATOR "${CPACK_GENERATOR};DragNDrop")

    set(CPACK_DMG_DISABLE_APPLICATIONS_SYMLINK TRUE)
endif()

set(CPACK_SET_DESTDIR ON)

set(CPACK_PACKAGE_CONTACT "Linus Dierheimer <Linus@Dierheimer.de>")
set(CPACK_PACKAGE_DESCRIPTION "\
fastfetch is a neofetch-like tool for fetching system information and displaying them in a pretty way. \
It is written in c to achieve much better performance.\
")

include(CPack)

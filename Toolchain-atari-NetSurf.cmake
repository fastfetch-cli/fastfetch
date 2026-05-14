# This Toolchain file is used to cross compile the windows
# version of Hatari under linux using mingw32
# use : cmake -DCMAKE_TOOLCHAIN_FILE=Toolchain-atari.cmake

# The name of the target operating system
SET(CMAKE_SYSTEM_NAME MINT)

# MINT versions of the different tools
# (change these depending on your system settings)
SET(CMAKE_C_COMPILER m68k-atari-mint-gcc)
SET(CMAKE_CXX_COMPILER m68k-atari-mint-g++)
SET(CMAKE_RC_COMPILER m68k-atari-mint-windres)
SET(CMAKE_ASM_COMPILER m68k-atari-mint-as)
SET(CMAKE_STRIP m68k-atari-mint-strip)

# Base directory for the target environment
SET(CMAKE_FIND_ROOT_PATH /opt/netsurf/m68k-atari-mint/env )

# FindSDL.cmake doesn't search correctly in CMAKE_FIND_ROOT_PATH
# so we force SDLDIR here
set ( ENV{SDLDIR} ${CMAKE_FIND_ROOT_PATH}/include/SDL )

# Adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search 
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

#
set(CMAKE_EXECUTABLE_SUFFIX .prg)
set(CMAKE_EXECUTABLE_SUFFIX_ASM .prg)
set(CMAKE_EXECUTABLE_SUFFIX_C .prg)
set(CMAKE_EXECUTABLE_SUFFIX_CXX .prg)

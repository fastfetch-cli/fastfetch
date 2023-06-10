#pragma once

// This file will be included in "fastfetch.h", do NOT put unnecessary things here

#include "common/option.h"

#if defined(__linux__) || defined(__FreeBSD__)
typedef enum FFOpenGLType
{
    FF_OPENGL_TYPE_AUTO,
    FF_OPENGL_TYPE_EGL,
    FF_OPENGL_TYPE_GLX,
    FF_OPENGL_TYPE_OSMESA
} FFOpenGLType;
#endif

typedef struct FFOpenGLOptions
{
    const char* moduleName;
    FFModuleArgs moduleArgs;

    #if defined(__linux__) || defined(__FreeBSD__)
    FFOpenGLType type;
    #endif
} FFOpenGLOptions;

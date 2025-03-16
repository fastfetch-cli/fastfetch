#pragma once

// This file will be included in "fastfetch.h", do NOT put unnecessary things here

#include "common/option.h"

typedef enum __attribute__((__packed__)) FFOpenGLLibrary
{
    FF_OPENGL_LIBRARY_AUTO,
    FF_OPENGL_LIBRARY_EGL,
    FF_OPENGL_LIBRARY_GLX,
} FFOpenGLLibrary;

typedef struct FFOpenGLOptions
{
    FFModuleBaseInfo moduleInfo;
    FFModuleArgs moduleArgs;

    FFOpenGLLibrary library;
} FFOpenGLOptions;

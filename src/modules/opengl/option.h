#pragma once

#include "common/option.h"

typedef enum __attribute__((__packed__)) FFOpenGLLibrary
{
    FF_OPENGL_LIBRARY_AUTO,
    FF_OPENGL_LIBRARY_EGL,
    FF_OPENGL_LIBRARY_GLX,
} FFOpenGLLibrary;

typedef struct FFOpenGLOptions
{
    FFModuleArgs moduleArgs;

    FFOpenGLLibrary library;
} FFOpenGLOptions;

static_assert(sizeof(FFOpenGLOptions) <= FF_OPTION_MAX_SIZE, "FFOpenGLOptions size exceeds maximum allowed size");

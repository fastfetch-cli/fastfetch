#pragma once

#include "fastfetch.h"

typedef struct FFOpenGLResult
{
    FFstrbuf version;
    FFstrbuf renderer;
    FFstrbuf vendor;
    FFstrbuf slv;
    FFstrbuf library;
} FFOpenGLResult;

#define FF_OPENGL_BUFFER_WIDTH 1
#define FF_OPENGL_BUFFER_HEIGHT 1

const char* ffDetectOpenGL(FFOpenGLOptions* options, FFOpenGLResult* result);

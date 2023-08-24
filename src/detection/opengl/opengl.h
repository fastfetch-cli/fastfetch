#pragma once

#ifndef FF_INCLUDED_detection_opengl_opengl
#define FF_INCLUDED_detection_opengl_opengl

#include "fastfetch.h"

typedef struct FFOpenGLResult
{
    FFstrbuf version;
    FFstrbuf renderer;
    FFstrbuf vendor;
    FFstrbuf slv;
} FFOpenGLResult;

const char* ffDetectOpenGL(FFOpenGLOptions* options, FFOpenGLResult* result);

#endif

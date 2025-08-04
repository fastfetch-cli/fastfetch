#pragma once

#include "option.h"

#define FF_OPENGL_MODULE_NAME "OpenGL"

void ffPrintOpenGL(FFOpenGLOptions* options);
void ffInitOpenGLOptions(FFOpenGLOptions* options);
void ffDestroyOpenGLOptions(FFOpenGLOptions* options);

extern FFModuleBaseInfo ffOpenGLModuleInfo;

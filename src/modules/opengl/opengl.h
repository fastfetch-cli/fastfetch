#pragma once

#include "fastfetch.h"
#include <yyjson.h>

#define FF_OPENGL_MODULE_NAME "OpenGL"

void ffPrintOpenGL(FFinstance* instance, FFOpenGLOptions* options);
void ffInitOpenGLOptions(FFOpenGLOptions* options);
bool ffParseOpenGLCommandOptions(FFOpenGLOptions* options, const char* key, const char* value);
void ffDestroyOpenGLOptions(FFOpenGLOptions* options);
void ffParseOpenGLJsonObject(FFinstance* instance, yyjson_val* module);

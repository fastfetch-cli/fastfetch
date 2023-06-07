#pragma once

#include "fastfetch.h"

#define FF_OPENGL_MODULE_NAME "OpenGL"

void ffPrintOpenGL(FFinstance* instance, FFOpenGLOptions* options);
void ffInitOpenGLOptions(FFOpenGLOptions* options);
bool ffParseOpenGLCommandOptions(FFOpenGLOptions* options, const char* key, const char* value);
void ffDestroyOpenGLOptions(FFOpenGLOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseOpenGLJsonObject(FFinstance* instance, json_object* module);
#endif

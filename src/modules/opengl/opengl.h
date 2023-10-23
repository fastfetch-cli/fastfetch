#pragma once

#include "fastfetch.h"

#define FF_OPENGL_MODULE_NAME "OpenGL"

void ffPrintOpenGL(FFOpenGLOptions* options);
void ffInitOpenGLOptions(FFOpenGLOptions* options);
bool ffParseOpenGLCommandOptions(FFOpenGLOptions* options, const char* key, const char* value);
void ffDestroyOpenGLOptions(FFOpenGLOptions* options);
void ffParseOpenGLJsonObject(FFOpenGLOptions* options, yyjson_val* module);
void ffGenerateOpenGLJsonResult(FFOpenGLOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module);
void ffPrintOpenGLHelpFormat(void);

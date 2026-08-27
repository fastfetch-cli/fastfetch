#pragma once

#include "option.h"

bool ffPrintOpenGL(FFOpenGLOptions* options);
void ffInitOpenGLOptions(FFOpenGLOptions* options);
void ffDestroyOpenGLOptions(FFOpenGLOptions* options);

extern FFModuleBaseInfo ffOpenGLModuleInfo;

#pragma once

#include "fastfetch.h"

#define FF_DOTFILEMANAGER_MODULE_NAME "DotfileManager"

void ffPrintDotfileManager(FFDotfileManagerOptions* options);
void ffInitDotfileManagerOptions(FFDotfileManagerOptions* options);
void ffDestroyDotfileManagerOptions(FFDotfileManagerOptions* options);

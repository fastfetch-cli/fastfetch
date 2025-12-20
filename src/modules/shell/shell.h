#pragma once

#include "option.h"

#define FF_SHELL_MODULE_NAME "Shell"

bool ffPrintShell(FFShellOptions* options);
void ffInitShellOptions(FFShellOptions* options);
void ffDestroyShellOptions(FFShellOptions* options);

extern FFModuleBaseInfo ffShellModuleInfo;

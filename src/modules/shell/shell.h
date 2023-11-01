#pragma once

#include "fastfetch.h"

#define FF_SHELL_MODULE_NAME "Shell"

void ffPrintShell(FFShellOptions* options);
void ffInitShellOptions(FFShellOptions* options);
void ffDestroyShellOptions(FFShellOptions* options);

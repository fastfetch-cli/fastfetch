#pragma once

#include "option.h"

bool ffPrintShell(FFShellOptions* options);
void ffInitShellOptions(FFShellOptions* options);
void ffDestroyShellOptions(FFShellOptions* options);

extern FFModuleBaseInfo ffShellModuleInfo;

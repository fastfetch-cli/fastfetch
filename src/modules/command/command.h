#pragma once

#include "option.h"

#define FF_COMMAND_MODULE_NAME "Command"

bool ffPrintCommand(FFCommandOptions* options);
void ffInitCommandOptions(FFCommandOptions* options);
void ffDestroyCommandOptions(FFCommandOptions* options);

extern FFModuleBaseInfo ffCommandModuleInfo;

#pragma once

#include "fastfetch.h"

#define FF_COMMAND_MODULE_NAME "Command"

void ffPrintCommand(FFCommandOptions* options);
void ffInitCommandOptions(FFCommandOptions* options);
void ffDestroyCommandOptions(FFCommandOptions* options);

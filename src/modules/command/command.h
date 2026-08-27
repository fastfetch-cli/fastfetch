#pragma once

#include "option.h"

bool ffPrepareCommand(FFCommandOptions* options);

bool ffPrintCommand(FFCommandOptions* options);
void ffInitCommandOptions(FFCommandOptions* options);
void ffDestroyCommandOptions(FFCommandOptions* options);

extern FFModuleBaseInfo ffCommandModuleInfo;

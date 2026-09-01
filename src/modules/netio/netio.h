#pragma once

#include "option.h"

void ffPrepareNetIO(FFNetIOOptions* options);

bool ffPrintNetIO(FFNetIOOptions* options);
void ffInitNetIOOptions(FFNetIOOptions* options);
void ffDestroyNetIOOptions(FFNetIOOptions* options);

extern FFModuleBaseInfo ffNetIOModuleInfo;

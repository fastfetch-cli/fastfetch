#pragma once

#include "option.h"

#define FF_NETIO_MODULE_NAME "NetIO"

void ffPrepareNetIO(FFNetIOOptions* options);

bool ffPrintNetIO(FFNetIOOptions* options);
void ffInitNetIOOptions(FFNetIOOptions* options);
void ffDestroyNetIOOptions(FFNetIOOptions* options);

extern FFModuleBaseInfo ffNetIOModuleInfo;

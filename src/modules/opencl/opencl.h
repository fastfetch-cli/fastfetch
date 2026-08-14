#pragma once

#include "option.h"

bool ffPrintOpenCL(FFOpenCLOptions* options);
void ffInitOpenCLOptions(FFOpenCLOptions* options);
void ffDestroyOpenCLOptions(FFOpenCLOptions* options);

extern FFModuleBaseInfo ffOpenCLModuleInfo;

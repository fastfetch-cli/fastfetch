#pragma once

#include "option.h"

#define FF_WM_MODULE_NAME "WM"

bool ffPrintWM(FFWMOptions* options);
void ffInitWMOptions(FFWMOptions* options);
void ffDestroyWMOptions(FFWMOptions* options);

extern FFModuleBaseInfo ffWMModuleInfo;

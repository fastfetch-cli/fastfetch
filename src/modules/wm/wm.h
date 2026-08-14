#pragma once

#include "option.h"

bool ffPrintWM(FFWMOptions* options);
void ffInitWMOptions(FFWMOptions* options);
void ffDestroyWMOptions(FFWMOptions* options);

extern FFModuleBaseInfo ffWMModuleInfo;

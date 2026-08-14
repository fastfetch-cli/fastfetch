#pragma once

#include "option.h"

bool ffPrintGamepad(FFGamepadOptions* options);
void ffInitGamepadOptions(FFGamepadOptions* options);
void ffDestroyGamepadOptions(FFGamepadOptions* options);

extern FFModuleBaseInfo ffGamepadModuleInfo;

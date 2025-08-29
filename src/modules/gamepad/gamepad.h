#pragma once

#include "option.h"

#define FF_GAMEPAD_MODULE_NAME "Gamepad"

bool ffPrintGamepad(FFGamepadOptions* options);
void ffInitGamepadOptions(FFGamepadOptions* options);
void ffDestroyGamepadOptions(FFGamepadOptions* options);

extern FFModuleBaseInfo ffGamepadModuleInfo;

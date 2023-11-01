#pragma once

#include "fastfetch.h"

#define FF_GAMEPAD_MODULE_NAME "Gamepad"

void ffPrintGamepad(FFGamepadOptions* options);
void ffInitGamepadOptions(FFGamepadOptions* options);
void ffDestroyGamepadOptions(FFGamepadOptions* options);

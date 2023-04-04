#pragma once

#include "fastfetch.h"

#define FF_GAMEPAD_MODULE_NAME "Gamepad"

void ffPrintGamepad(FFinstance* instance, FFGamepadOptions* options);
void ffInitGamepadOptions(FFGamepadOptions* options);
bool ffParseGamepadCommandOptions(FFGamepadOptions* options, const char* key, const char* value);
void ffDestroyGamepadOptions(FFGamepadOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseGamepadJsonObject(FFinstance* instance, json_object* module);
#endif

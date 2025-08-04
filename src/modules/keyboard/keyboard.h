#pragma once

#include "option.h"

#define FF_KEYBOARD_MODULE_NAME "Keyboard"

void ffPrintKeyboard(FFKeyboardOptions* options);
void ffInitKeyboardOptions(FFKeyboardOptions* options);
void ffDestroyKeyboardOptions(FFKeyboardOptions* options);

extern FFModuleBaseInfo ffKeyboardModuleInfo;

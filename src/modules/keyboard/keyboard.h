#pragma once

#include "fastfetch.h"

#define FF_KEYBOARD_MODULE_NAME "Keyboard"

void ffPrintKeyboard(FFKeyboardOptions* options);
void ffInitKeyboardOptions(FFKeyboardOptions* options);
void ffDestroyKeyboardOptions(FFKeyboardOptions* options);

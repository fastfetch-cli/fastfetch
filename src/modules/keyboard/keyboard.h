#pragma once

#include "option.h"

bool ffPrintKeyboard(FFKeyboardOptions* options);
void ffInitKeyboardOptions(FFKeyboardOptions* options);
void ffDestroyKeyboardOptions(FFKeyboardOptions* options);

extern FFModuleBaseInfo ffKeyboardModuleInfo;

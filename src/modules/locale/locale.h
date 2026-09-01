#pragma once

#include "option.h"

bool ffPrintLocale(FFLocaleOptions* options);
void ffInitLocaleOptions(FFLocaleOptions* options);
void ffDestroyLocaleOptions(FFLocaleOptions* options);

extern FFModuleBaseInfo ffLocaleModuleInfo;

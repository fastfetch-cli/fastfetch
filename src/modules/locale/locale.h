#pragma once

#include "option.h"

#define FF_LOCALE_MODULE_NAME "Locale"

bool ffPrintLocale(FFLocaleOptions* options);
void ffInitLocaleOptions(FFLocaleOptions* options);
void ffDestroyLocaleOptions(FFLocaleOptions* options);

extern FFModuleBaseInfo ffLocaleModuleInfo;

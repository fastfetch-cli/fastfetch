#pragma once

#include "fastfetch.h"

#define FF_LOCALE_MODULE_NAME "Locale"

void ffPrintLocale(FFLocaleOptions* options);
void ffInitLocaleOptions(FFLocaleOptions* options);
void ffDestroyLocaleOptions(FFLocaleOptions* options);

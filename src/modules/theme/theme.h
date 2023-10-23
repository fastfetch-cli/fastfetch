#pragma once

#include "fastfetch.h"

#define FF_THEME_MODULE_NAME "Theme"

void ffPrintTheme(FFThemeOptions* options);
void ffInitThemeOptions(FFThemeOptions* options);
void ffDestroyThemeOptions(FFThemeOptions* options);

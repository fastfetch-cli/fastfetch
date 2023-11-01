#pragma once

#include "fastfetch.h"

#define FF_BRIGHTNESS_MODULE_NAME "Brightness"

void ffPrintBrightness(FFBrightnessOptions* options);
void ffInitBrightnessOptions(FFBrightnessOptions* options);
void ffDestroyBrightnessOptions(FFBrightnessOptions* options);

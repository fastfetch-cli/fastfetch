#pragma once

#include "option.h"

#define FF_BRIGHTNESS_MODULE_NAME "Brightness"

bool ffPrintBrightness(FFBrightnessOptions* options);
void ffInitBrightnessOptions(FFBrightnessOptions* options);
void ffDestroyBrightnessOptions(FFBrightnessOptions* options);

extern FFModuleBaseInfo ffBrightnessModuleInfo;

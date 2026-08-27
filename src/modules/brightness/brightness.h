#pragma once

#include "option.h"

bool ffPrintBrightness(FFBrightnessOptions* options);
void ffInitBrightnessOptions(FFBrightnessOptions* options);
void ffDestroyBrightnessOptions(FFBrightnessOptions* options);

extern FFModuleBaseInfo ffBrightnessModuleInfo;

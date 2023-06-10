#pragma once

#include "fastfetch.h"

#define FF_BRIGHTNESS_MODULE_NAME "Brightness"

void ffPrintBrightness(FFinstance* instance, FFBrightnessOptions* options);
void ffInitBrightnessOptions(FFBrightnessOptions* options);
bool ffParseBrightnessCommandOptions(FFBrightnessOptions* options, const char* key, const char* value);
void ffDestroyBrightnessOptions(FFBrightnessOptions* options);
void ffParseBrightnessJsonObject(FFinstance* instance, yyjson_val* module);

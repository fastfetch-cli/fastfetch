#pragma once

#include "fastfetch.h"

#define FF_BRIGHTNESS_MODULE_NAME "Brightness"

void ffPrintBrightness(FFinstance* instance, FFBrightnessOptions* options);
void ffInitBrightnessOptions(FFBrightnessOptions* options);
bool ffParseBrightnessCommandOptions(FFBrightnessOptions* options, const char* key, const char* value);
void ffDestroyBrightnessOptions(FFBrightnessOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseBrightnessJsonObject(FFinstance* instance, json_object* module);
#endif

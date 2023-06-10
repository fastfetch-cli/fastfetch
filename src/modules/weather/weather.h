#pragma once

#include "fastfetch.h"

#define FF_WEATHER_MODULE_NAME "Weather"

void ffPrepareWeather(FFWeatherOptions* options);

void ffPrintWeather(FFinstance* instance, FFWeatherOptions* options);
void ffInitWeatherOptions(FFWeatherOptions* options);
bool ffParseWeatherCommandOptions(FFWeatherOptions* options, const char* key, const char* value);
void ffDestroyWeatherOptions(FFWeatherOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseWeatherJsonObject(FFinstance* instance, json_object* module);
#endif

#pragma once

#include "fastfetch.h"
#include <yyjson.h>

#define FF_WEATHER_MODULE_NAME "Weather"

void ffPrepareWeather(FFWeatherOptions* options);

void ffPrintWeather(FFinstance* instance, FFWeatherOptions* options);
void ffInitWeatherOptions(FFWeatherOptions* options);
bool ffParseWeatherCommandOptions(FFWeatherOptions* options, const char* key, const char* value);
void ffDestroyWeatherOptions(FFWeatherOptions* options);
void ffParseWeatherJsonObject(FFinstance* instance, yyjson_val* module);

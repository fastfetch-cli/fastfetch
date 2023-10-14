#pragma once

#include "fastfetch.h"

#define FF_WEATHER_MODULE_NAME "Weather"

void ffPrepareWeather(FFWeatherOptions* options);

void ffPrintWeather(FFWeatherOptions* options);
void ffInitWeatherOptions(FFWeatherOptions* options);
bool ffParseWeatherCommandOptions(FFWeatherOptions* options, const char* key, const char* value);
void ffDestroyWeatherOptions(FFWeatherOptions* options);
void ffParseWeatherJsonObject(FFWeatherOptions* options, yyjson_val* module);
void ffGenerateWeatherJson(FFWeatherOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module);
void ffPrintWeatherHelpFormat(void);

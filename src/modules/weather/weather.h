#pragma once

#include "option.h"

#define FF_WEATHER_MODULE_NAME "Weather"

void ffPrepareWeather(FFWeatherOptions* options);

bool ffPrintWeather(FFWeatherOptions* options);
void ffInitWeatherOptions(FFWeatherOptions* options);
void ffDestroyWeatherOptions(FFWeatherOptions* options);

extern FFModuleBaseInfo ffWeatherModuleInfo;

#pragma once

#include "fastfetch.h"

#define FF_WEATHER_MODULE_NAME "Weather"

void ffPrepareWeather(FFWeatherOptions* options);

void ffPrintWeather(FFWeatherOptions* options);
void ffInitWeatherOptions(FFWeatherOptions* options);
void ffDestroyWeatherOptions(FFWeatherOptions* options);

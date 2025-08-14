#pragma once

#include "fastfetch.h"
#include "modules/weather/option.h"

void ffPrepareWeather(FFWeatherOptions* options);
const char* ffDetectWeather(FFWeatherOptions* options, FFstrbuf* result);

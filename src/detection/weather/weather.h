#pragma once

#include "fastfetch.h"

void ffPrepareWeather(FFWeatherOptions* options);
const char* ffDetectWeather(FFWeatherOptions* options, FFstrbuf* result);

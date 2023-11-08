#pragma once

#include "common/networking.h"

void ffPrepareWeather(FFWeatherOptions* options);
const char* ffDetectWeather(FFWeatherOptions* options, FFstrbuf* result);

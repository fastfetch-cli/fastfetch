#pragma once

#ifndef FF_INCLUDED_detection_weather_weather
#define FF_INCLUDED_detection_weather_weather

#include "common/networking.h"

void ffPrepareWeather(FFWeatherOptions* options);
const char* ffDetectWeather(FFWeatherOptions* options, FFstrbuf* result);

#endif

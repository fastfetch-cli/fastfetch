#pragma once

#include "option.h"

void ffPrepareWeather(FFWeatherOptions* options);

bool ffPrintWeather(FFWeatherOptions* options);
void ffInitWeatherOptions(FFWeatherOptions* options);
void ffDestroyWeatherOptions(FFWeatherOptions* options);

extern FFModuleBaseInfo ffWeatherModuleInfo;

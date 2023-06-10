#pragma once

#include "fastfetch.h"
#include <yyjson.h>

#define FF_CHASSIS_MODULE_NAME "Chassis"

void ffPrintChassis(FFinstance* instance, FFChassisOptions* options);
void ffInitChassisOptions(FFChassisOptions* options);
bool ffParseChassisCommandOptions(FFChassisOptions* options, const char* key, const char* value);
void ffDestroyChassisOptions(FFChassisOptions* options);
void ffParseChassisJsonObject(FFinstance* instance, yyjson_val* module);

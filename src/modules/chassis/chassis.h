#pragma once

#include "fastfetch.h"

#define FF_CHASSIS_MODULE_NAME "Chassis"

void ffPrintChassis(FFinstance* instance, FFChassisOptions* options);
void ffInitChassisOptions(FFChassisOptions* options);
bool ffParseChassisCommandOptions(FFChassisOptions* options, const char* key, const char* value);
void ffDestroyChassisOptions(FFChassisOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseChassisJsonObject(FFinstance* instance, json_object* module);
#endif

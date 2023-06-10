#pragma once

#include "fastfetch.h"
#include "modules/title/option.h"

#define FF_UPTIME_MODULE_NAME "Uptime"

void ffPrintUptime(FFinstance* instance, FFUptimeOptions* options);
void ffInitUptimeOptions(FFUptimeOptions* options);
bool ffParseUptimeCommandOptions(FFUptimeOptions* options, const char* key, const char* value);
void ffDestroyUptimeOptions(FFUptimeOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseUptimeJsonObject(FFinstance* instance, json_object* module);
#endif

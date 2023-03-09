#pragma once

#include "fastfetch.h"
#include "modules/datetime/option.h"

void ffPrintDateTime(FFinstance* instance, FFDateTimeOptions* options);
void ffInitDateTimeOptions(FFDateTimeOptions* options);
bool ffParseDateTimeCommandOptions(FFDateTimeOptions* options, const char* key, const char* value);
void ffDestroyDateTimeOptions(FFDateTimeOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/config.h"
bool ffParseDateTimeJsonObject(FFinstance* instance, const char* type, JSONCData* data, json_object* module);
#endif

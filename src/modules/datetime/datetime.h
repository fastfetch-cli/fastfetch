#pragma once

#include "fastfetch.h"

#define FF_DATETIME_MODULE_NAME "DateTime"

void ffPrintDateTime(FFinstance* instance, FFDateTimeOptions* options);
void ffInitDateTimeOptions(FFDateTimeOptions* options);
bool ffParseDateTimeCommandOptions(FFDateTimeOptions* options, const char* key, const char* value);
void ffDestroyDateTimeOptions(FFDateTimeOptions* options);
void ffParseDateTimeJsonObject(FFinstance* instance, yyjson_val* module);

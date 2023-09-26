#pragma once

#include "fastfetch.h"

#define FF_NETUSAGE_MODULE_NAME "NetUsage"

void ffPrepareNetUsage();

void ffPrintNetUsage(FFNetUsageOptions* options);
void ffInitNetUsageOptions(FFNetUsageOptions* options);
bool ffParseNetUsageCommandOptions(FFNetUsageOptions* options, const char* key, const char* value);
void ffDestroyNetUsageOptions(FFNetUsageOptions* options);
void ffParseNetUsageJsonObject(FFNetUsageOptions* options, yyjson_val* module);
void ffGenerateNetUsageJson(FFNetUsageOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module);

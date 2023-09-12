#pragma once

#include "fastfetch.h"

#define FF_VERSION_MODULE_NAME "Version"

void ffPrintVersion(FFVersionOptions* options);
void ffInitVersionOptions(FFVersionOptions* options);
bool ffParseVersionCommandOptions(FFVersionOptions* options, const char* key, const char* value);
void ffDestroyVersionOptions(FFVersionOptions* options);
void ffParseVersionJsonObject(FFVersionOptions* options, yyjson_val* module);
void ffGenerateVersionJson(FFVersionOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module);

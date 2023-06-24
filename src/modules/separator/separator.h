#pragma once

#include "fastfetch.h"

#define FF_SEPARATOR_MODULE_NAME "Separator"

void ffPrintSeparator(FFSeparatorOptions* options);
void ffInitSeparatorOptions(FFSeparatorOptions* options);
bool ffParseSeparatorCommandOptions(FFSeparatorOptions* options, const char* key, const char* value);
void ffDestroySeparatorOptions(FFSeparatorOptions* options);
void ffParseSeparatorJsonObject(yyjson_val* module);

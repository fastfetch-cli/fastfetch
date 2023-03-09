#pragma once

#include "fastfetch.h"
#include "modules/separator/option.h"

void ffPrintSeparator(FFinstance* instance, FFSeparatorOptions* options);
void ffInitSeparatorOptions(FFSeparatorOptions* options);
bool ffParseSeparatorCommandOptions(FFSeparatorOptions* options, const char* key, const char* value);
void ffDestroySeparatorOptions(FFSeparatorOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/config.h"
bool ffParseSeparatorJsonObject(FFinstance* instance, const char* type, JSONCData* data, json_object* module);
#endif

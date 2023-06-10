#pragma once

#include "fastfetch.h"
#include "modules/separator/option.h"

#define FF_SEPARATOR_MODULE_NAME "Separator"

void ffPrintSeparator(FFinstance* instance, FFSeparatorOptions* options);
void ffInitSeparatorOptions(FFSeparatorOptions* options);
bool ffParseSeparatorCommandOptions(FFSeparatorOptions* options, const char* key, const char* value);
void ffDestroySeparatorOptions(FFSeparatorOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseSeparatorJsonObject(FFinstance* instance, json_object* module);
#endif

#pragma once

#include "fastfetch.h"

#define FF_DE_MODULE_NAME "DE"

void ffPrintDE(FFinstance* instance, FFDEOptions* options);
void ffInitDEOptions(FFDEOptions* options);
bool ffParseDECommandOptions(FFDEOptions* options, const char* key, const char* value);
void ffDestroyDEOptions(FFDEOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseDEJsonObject(FFinstance* instance, json_object* module);
#endif

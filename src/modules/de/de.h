#pragma once

#include "fastfetch.h"
#include <yyjson.h>

#define FF_DE_MODULE_NAME "DE"

void ffPrintDE(FFinstance* instance, FFDEOptions* options);
void ffInitDEOptions(FFDEOptions* options);
bool ffParseDECommandOptions(FFDEOptions* options, const char* key, const char* value);
void ffDestroyDEOptions(FFDEOptions* options);
void ffParseDEJsonObject(FFinstance* instance, yyjson_val* module);

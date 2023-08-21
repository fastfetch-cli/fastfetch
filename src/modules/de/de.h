#pragma once

#include "fastfetch.h"

#define FF_DE_MODULE_NAME "DE"

void ffPrintDE(FFDEOptions* options);
void ffInitDEOptions(FFDEOptions* options);
bool ffParseDECommandOptions(FFDEOptions* options, const char* key, const char* value);
void ffDestroyDEOptions(FFDEOptions* options);
void ffParseDEJsonObject(FFDEOptions* options, yyjson_val* module);

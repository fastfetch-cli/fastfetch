#pragma once

#include "fastfetch.h"

#define FF_DE_MODULE_NAME "DE"

void ffPrintDE(FFDEOptions* options);
void ffInitDEOptions(FFDEOptions* options);
bool ffParseDECommandOptions(FFDEOptions* options, const char* key, const char* value);
void ffDestroyDEOptions(FFDEOptions* options);
void ffParseDEJsonObject(FFDEOptions* options, yyjson_val* module);
void ffGenerateDEJsonResult(FFDEOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module);
void ffPrintDEHelpFormat(void);

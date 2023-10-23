#pragma once

#include "fastfetch.h"

#define FF_NETIO_MODULE_NAME "NetIO"

void ffPrepareNetIO(FFNetIOOptions* options);

void ffPrintNetIO(FFNetIOOptions* options);
void ffInitNetIOOptions(FFNetIOOptions* options);
bool ffParseNetIOCommandOptions(FFNetIOOptions* options, const char* key, const char* value);
void ffDestroyNetIOOptions(FFNetIOOptions* options);
void ffParseNetIOJsonObject(FFNetIOOptions* options, yyjson_val* module);
void ffGenerateNetIOJsonResult(FFNetIOOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module);
void ffPrintNetIOHelpFormat(void);

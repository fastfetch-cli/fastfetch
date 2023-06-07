#pragma once

#include "fastfetch.h"

#define FF_OPENCL_MODULE_NAME "OpenCL"

void ffPrintOpenCL(FFinstance* instance, FFOpenCLOptions* options);
void ffInitOpenCLOptions(FFOpenCLOptions* options);
bool ffParseOpenCLCommandOptions(FFOpenCLOptions* options, const char* key, const char* value);
void ffDestroyOpenCLOptions(FFOpenCLOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseOpenCLJsonObject(FFinstance* instance, json_object* module);
#endif

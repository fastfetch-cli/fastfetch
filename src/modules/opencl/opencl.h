#pragma once

#include "fastfetch.h"

#define FF_OPENCL_MODULE_NAME "OpenCL"

void ffPrintOpenCL(FFOpenCLOptions* options);
void ffInitOpenCLOptions(FFOpenCLOptions* options);
bool ffParseOpenCLCommandOptions(FFOpenCLOptions* options, const char* key, const char* value);
void ffDestroyOpenCLOptions(FFOpenCLOptions* options);
void ffParseOpenCLJsonObject(FFOpenCLOptions* options, yyjson_val* module);
void ffGenerateOpenCLJsonResult(FFOpenCLOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module);
void ffPrintOpenCLHelpFormat(void);

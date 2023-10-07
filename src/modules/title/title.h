#pragma once

#include "fastfetch.h"

#define FF_TITLE_MODULE_NAME "Title"

void ffPrintTitle(FFTitleOptions* options);
void ffInitTitleOptions(FFTitleOptions* options);
bool ffParseTitleCommandOptions(FFTitleOptions* options, const char* key, const char* value);
void ffDestroyTitleOptions(FFTitleOptions* options);
void ffParseTitleJsonObject(FFTitleOptions* options, yyjson_val* module);
void ffGenerateTitleJson(FFTitleOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module);
void ffPrintTitleHelpFormat(void);

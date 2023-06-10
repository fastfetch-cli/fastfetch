#pragma once

#include "fastfetch.h"
#include <yyjson.h>

#define FF_TITLE_MODULE_NAME "Title"

void ffPrintTitle(FFinstance* instance, FFTitleOptions* options);
void ffInitTitleOptions(FFTitleOptions* options);
bool ffParseTitleCommandOptions(FFTitleOptions* options, const char* key, const char* value);
void ffDestroyTitleOptions(FFTitleOptions* options);
void ffParseTitleJsonObject(FFinstance* instance, yyjson_val* module);

#pragma once

#include "fastfetch.h"
#include "modules/title/option.h"

#define FF_TITLE_MODULE_NAME "Title"

void ffPrintTitle(FFinstance* instance, FFTitleOptions* options);
void ffInitTitleOptions(FFTitleOptions* options);
bool ffParseTitleCommandOptions(FFTitleOptions* options, const char* key, const char* value);
void ffDestroyTitleOptions(FFTitleOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseTitleJsonObject(FFinstance* instance, json_object* module);
#endif

#pragma once

#include "fastfetch.h"

#define FF_FONT_MODULE_NAME "Font"

void ffPrintFont(FFinstance* instance, FFFontOptions* options);
void ffInitFontOptions(FFFontOptions* options);
bool ffParseFontCommandOptions(FFFontOptions* options, const char* key, const char* value);
void ffDestroyFontOptions(FFFontOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseFontJsonObject(FFinstance* instance, json_object* module);
#endif

#pragma once

#include "fastfetch.h"
#include <yyjson.h>

#define FF_FONT_MODULE_NAME "Font"

void ffPrintFont(FFinstance* instance, FFFontOptions* options);
void ffInitFontOptions(FFFontOptions* options);
bool ffParseFontCommandOptions(FFFontOptions* options, const char* key, const char* value);
void ffDestroyFontOptions(FFFontOptions* options);
void ffParseFontJsonObject(FFinstance* instance, yyjson_val* module);

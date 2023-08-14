#pragma once

#include "fastfetch.h"

#define FF_FONT_MODULE_NAME "Font"

void ffPrintFont(FFFontOptions* options);
void ffInitFontOptions(FFFontOptions* options);
bool ffParseFontCommandOptions(FFFontOptions* options, const char* key, const char* value);
void ffDestroyFontOptions(FFFontOptions* options);
void ffParseFontJsonObject(yyjson_val* module);

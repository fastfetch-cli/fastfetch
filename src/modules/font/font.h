#pragma once

#include "option.h"

#define FF_FONT_MODULE_NAME "Font"

bool ffPrintFont(FFFontOptions* options);
void ffInitFontOptions(FFFontOptions* options);
void ffDestroyFontOptions(FFFontOptions* options);

extern FFModuleBaseInfo ffFontModuleInfo;

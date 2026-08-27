#pragma once

#include "option.h"

bool ffPrintFont(FFFontOptions* options);
void ffInitFontOptions(FFFontOptions* options);
void ffDestroyFontOptions(FFFontOptions* options);

extern FFModuleBaseInfo ffFontModuleInfo;

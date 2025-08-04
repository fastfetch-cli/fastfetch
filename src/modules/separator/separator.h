#pragma once

#include "option.h"

#define FF_SEPARATOR_MODULE_NAME "Separator"

void ffPrintSeparator(FFSeparatorOptions* options);
void ffInitSeparatorOptions(FFSeparatorOptions* options);
void ffDestroySeparatorOptions(FFSeparatorOptions* options);

extern FFModuleBaseInfo ffSeparatorModuleInfo;

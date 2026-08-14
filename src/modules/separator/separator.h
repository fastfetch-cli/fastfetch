#pragma once

#include "option.h"

bool ffPrintSeparator(FFSeparatorOptions* options);
void ffInitSeparatorOptions(FFSeparatorOptions* options);
void ffDestroySeparatorOptions(FFSeparatorOptions* options);

extern FFModuleBaseInfo ffSeparatorModuleInfo;

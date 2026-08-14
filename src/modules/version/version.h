#pragma once

#include "option.h"

bool ffPrintVersion(FFVersionOptions* options);
void ffInitVersionOptions(FFVersionOptions* options);
void ffDestroyVersionOptions(FFVersionOptions* options);

extern FFModuleBaseInfo ffVersionModuleInfo;

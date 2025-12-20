#pragma once

#include "option.h"

#define FF_VERSION_MODULE_NAME "Version"

bool ffPrintVersion(FFVersionOptions* options);
void ffInitVersionOptions(FFVersionOptions* options);
void ffDestroyVersionOptions(FFVersionOptions* options);

extern FFModuleBaseInfo ffVersionModuleInfo;

#pragma once

#include "option.h"

bool ffPrintCPUCache(FFCPUCacheOptions* options);
void ffInitCPUCacheOptions(FFCPUCacheOptions* options);
void ffDestroyCPUCacheOptions(FFCPUCacheOptions* options);

extern FFModuleBaseInfo ffCPUCacheModuleInfo;

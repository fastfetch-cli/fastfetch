#pragma once

#include "option.h"

#define FF_CPUCACHE_MODULE_NAME "CPUCache"

bool ffPrintCPUCache(FFCPUCacheOptions* options);
void ffInitCPUCacheOptions(FFCPUCacheOptions* options);
void ffDestroyCPUCacheOptions(FFCPUCacheOptions* options);

extern FFModuleBaseInfo ffCPUCacheModuleInfo;

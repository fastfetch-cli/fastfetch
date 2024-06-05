#pragma once

#include "fastfetch.h"

#define FF_CPUCACHE_MODULE_NAME "CPUCache"

void ffPrintCPUCache(FFCPUCacheOptions* options);
void ffInitCPUCacheOptions(FFCPUCacheOptions* options);
void ffDestroyCPUCacheOptions(FFCPUCacheOptions* options);

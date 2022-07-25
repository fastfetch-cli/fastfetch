#pragma once

#ifndef FF_INCLUDED_common_caching
#define FF_INCLUDED_common_caching

#include "fastfetch.h"
#include "common/format.h"

typedef struct FFcache
{
    FILE* value;
    FILE* split;
} FFcache;

void ffCacheValidate(FFinstance* instance);

void ffCacheOpenWrite(FFinstance* instance, const char* moduleName, FFcache* cache);
void ffCacheClose(FFcache* cache);

bool ffPrintFromCache(FFinstance* instance, const char* moduleName, const FFModuleArgs* moduleArgs, uint32_t numArgs);
void ffPrintAndAppendToCache(FFinstance* instance, const char* moduleName, uint8_t moduleIndex, const FFModuleArgs* moduleArgs, FFcache* cache, const FFstrbuf* value, uint32_t numArgs, const FFformatarg* arguments);
void ffPrintAndWriteToCache(FFinstance* instance, const char* moduleName, const FFModuleArgs* moduleArgs, const FFstrbuf* value, uint32_t numArgs, const FFformatarg* arguments);

void ffCachingWriteData(const FFinstance* instance, const char* moduleName, size_t dataSize, const void* data);
bool ffCachingReadData(const FFinstance* instance, const char* moduleName, size_t dataSize, void* data);

#endif

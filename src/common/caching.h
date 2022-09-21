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

typedef void FFCache;

typedef bool(*FFCacheMethodStrbuf)(FFCache* cache, FFstrbuf* strbuf);
typedef bool(*FFCacheMethodData)(FFCache* cache, size_t dataSize, void* data);
typedef bool(*FFCacheMethodCallback)(void* data, FFCache* cache, FFCacheMethodStrbuf strbufMethod, FFCacheMethodData dataMethod);

bool ffCacheRead(const FFinstance* instance, void* obj, const char* cacheName, FFCacheMethodCallback callback);
void ffCacheWrite(const FFinstance* instance, void* obj, const char* cacheName, FFCacheMethodCallback callback);

#endif

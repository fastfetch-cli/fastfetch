#include "fastfetch.h"
#include "common/caching.h"
#include "common/io.h"
#include "common/printing.h"

#include <stdlib.h>
#include <string.h>

#define FF_CACHE_VERSION_NAME "cacheversion"
#define FF_CACHE_VERSION_EXTENSION "ffv"

#define FF_CACHE_VALUE_EXTENSION "ffcv"
#define FF_CACHE_SPLIT_EXTENSION "ffcs"

#define FF_CACHE_EXTENSION_V1 "ffc1"

static void getCacheFilePath(const FFinstance* instance, const char* moduleName, const char* extension, FFstrbuf* buffer)
{
    ffStrbufAppend(buffer, &instance->state.cacheDir);
    ffStrbufAppendS(buffer, moduleName);

    if(extension != NULL)
    {
        ffStrbufAppendC(buffer, '.');
        ffStrbufAppendS(buffer, extension);
    }
}

static void readCacheFile(FFinstance* instance, const char* moduleName, const char* extension, FFstrbuf* buffer)
{
    FFstrbuf path;
    ffStrbufInitA(&path, 64);
    getCacheFilePath(instance, moduleName, extension, &path);
    ffAppendFileBuffer(path.chars, buffer);
    ffStrbufDestroy(&path);
}

static void writeCacheFile(FFinstance* instance, const char* moduleName, const char* extension, FFstrbuf* content)
{
    FFstrbuf path;
    ffStrbufInitA(&path, 64);
    getCacheFilePath(instance, moduleName, extension, &path);
    ffWriteFileBuffer(path.chars, content);
    ffStrbufDestroy(&path);
}

void ffCacheValidate(FFinstance* instance)
{
    FFstrbuf content;
    ffStrbufInit(&content);
    readCacheFile(instance, FF_CACHE_VERSION_NAME, FF_CACHE_VERSION_EXTENSION, &content);

    const char exactVersion[] = FASTFETCH_PROJECT_VERSION FASTFETCH_PROJECT_VERSION_TWEAK;

    bool isSameVersion = ffStrbufCompS(&content, exactVersion) == 0;
    ffStrbufDestroy(&content);
    if(isSameVersion)
        return;

    instance->config.recache = true;

    FFstrbuf version;
    ffStrbufInitA(&version, sizeof(exactVersion));
    ffStrbufAppendS(&version, exactVersion);
    writeCacheFile(instance, FF_CACHE_VERSION_NAME, FF_CACHE_VERSION_EXTENSION, &version);
    ffStrbufDestroy(&version);
}

void ffCacheOpenWrite(FFinstance* instance, const char* moduleName, FFcache* cache)
{
    FFstrbuf cacheFileValue;
    ffStrbufInitA(&cacheFileValue, 64);
    getCacheFilePath(instance, moduleName, FF_CACHE_VALUE_EXTENSION, &cacheFileValue);
    cache->value = fopen(cacheFileValue.chars, "w");
    ffStrbufDestroy(&cacheFileValue);

    FFstrbuf cacheFileSplit;
    ffStrbufInitA(&cacheFileSplit, 64);
    getCacheFilePath(instance, moduleName, FF_CACHE_SPLIT_EXTENSION, &cacheFileSplit);
    cache->split = fopen(cacheFileSplit.chars, "w");
    ffStrbufDestroy(&cacheFileSplit);
}

void ffCacheClose(FFcache* cache)
{
    if(cache->value != NULL)
        fclose(cache->value);

    if(cache->split != NULL)
        fclose(cache->split);
}

static bool printCachedValue(FFinstance* instance, const char* moduleName, const FFModuleArgs* moduleArgs)
{
    FFstrbuf content;
    ffStrbufInitA(&content, 512);
    readCacheFile(instance, moduleName, FF_CACHE_VALUE_EXTENSION, &content);

    ffStrbufTrimRight(&content, '\0'); //Strbuf always appends a '\0' at the end. We want the last null byte to be at the position of the length

    if(content.length == 0)
        return false;

    uint8_t moduleCounter = 1;

    uint32_t startIndex = 0;
    while(startIndex < content.length)
    {
        uint32_t nullByteIndex = ffStrbufNextIndexC(&content, startIndex, '\0');
        uint8_t moduleIndex = (moduleCounter == 1 && nullByteIndex == content.length) ? 0 : moduleCounter;
        ffPrintLogoAndKey(instance, moduleName, moduleIndex, &moduleArgs->key);
        puts(content.chars + startIndex);
        startIndex = nullByteIndex + 1;
        ++moduleCounter;
    }

    ffStrbufDestroy(&content);

    return moduleCounter > 1;
}

static bool printCachedFormat(FFinstance* instance, const char* moduleName, const FFModuleArgs* moduleArgs, uint32_t numArgs)
{
    FFstrbuf content;
    ffStrbufInitA(&content, 512);
    readCacheFile(instance, moduleName, FF_CACHE_SPLIT_EXTENSION, &content);

    ffStrbufTrimRight(&content, '\0'); //Strbuf always appends a '\0' at the end. We want the last null byte to be at the position of the length

    if(content.length == 0)
        return false;

    uint8_t moduleCounter = 1;

    FFformatarg* arguments = calloc(numArgs, sizeof(FFformatarg));
    uint32_t argumentCounter = 0;

    uint32_t startIndex = 0;
    while(startIndex < content.length)
    {
        arguments[argumentCounter].type = FF_FORMAT_ARG_TYPE_STRING;
        arguments[argumentCounter].value = &content.chars[startIndex];
        ++argumentCounter;

        uint32_t nullByteIndex = ffStrbufNextIndexC(&content, startIndex, '\0');

        if(argumentCounter == numArgs)
        {
            uint8_t moduleIndex = (moduleCounter == 1 && nullByteIndex == content.length) ? 0 : moduleCounter;
            ffPrintFormat(instance, moduleName, moduleIndex, moduleArgs, numArgs, arguments);
            ++moduleCounter;
            argumentCounter = 0;
        }

        startIndex = nullByteIndex + 1;
    }

    free(arguments);
    ffStrbufDestroy(&content);

    return moduleCounter > 1;
}

bool ffPrintFromCache(FFinstance* instance, const char* moduleName, const FFModuleArgs* moduleArgs, uint32_t numArgs)
{
    if(instance->config.recache)
        return false;

    if(moduleArgs->outputFormat.length == 0)
        return printCachedValue(instance, moduleName, moduleArgs);
    else
        return printCachedFormat(instance, moduleName, moduleArgs, numArgs);
}

void ffPrintAndAppendToCache(FFinstance* instance, const char* moduleName, uint8_t moduleIndex, const FFModuleArgs* moduleArgs, FFcache* cache, const FFstrbuf* value, uint32_t numArgs, const FFformatarg* arguments)
{
    if(moduleArgs->outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, moduleName, moduleIndex, &moduleArgs->key);
        ffStrbufPutTo(value, stdout);
    }
    else
    {
        ffPrintFormat(instance, moduleName, moduleIndex, moduleArgs, numArgs, arguments);
    }

    if(cache->value != NULL)
    {
        ffStrbufWriteTo(value, cache->value);
        fputc('\0', cache->value);
    }

    if(cache->split == NULL)
        return;

    for(uint32_t i = 0; i < numArgs; i++)
    {
        FFstrbuf buffer;
        ffStrbufInitA(&buffer, 64);
        ffFormatAppendFormatArg(&buffer, &arguments[i]);
        ffStrbufWriteTo(&buffer, cache->split);
        ffStrbufDestroy(&buffer);
        fputc('\0', cache->split);
    }
}

void ffPrintAndWriteToCache(FFinstance* instance, const char* moduleName, const FFModuleArgs* moduleArgs, const FFstrbuf* value, uint32_t numArgs, const FFformatarg* arguments)
{
    FFcache cache;
    ffCacheOpenWrite(instance, moduleName, &cache);
    ffPrintAndAppendToCache(instance, moduleName, 0, moduleArgs, &cache, value, numArgs, arguments);
    ffCacheClose(&cache);
}

typedef struct FFCacheRead
{
    FFstrbuf data;
    uint32_t position;
} FFCacheRead;

static bool cacheReadStrbuf(FFCacheRead* cacheRead, FFstrbuf* strbuf)
{
    if(cacheRead->position >= cacheRead->data.length)
        return false;

    ffStrbufAppendS(strbuf, cacheRead->data.chars + cacheRead->position);
    cacheRead->position += strbuf->length + 1; // skip the null byte too
    return true;
}

static bool cacheReadData(FFCacheRead* cacheRead, size_t dataSize, void* data)
{
    if(cacheRead->position + dataSize > cacheRead->data.length)
        return false;

    memcpy(data, cacheRead->data.chars + cacheRead->position, dataSize);
    cacheRead->position += (uint32_t) dataSize;
    return true;
}

static bool cacheResetStrbuf(FFCache* cache, FFstrbuf* strbuf)
{
    FF_UNUSED(cache);
    ffStrbufClear(strbuf);
    return true;
}

static bool cacheResetData(FFCache* cacheRead, size_t dataSize, void* data)
{
    FF_UNUSED(cacheRead, dataSize, data);
    return true;
}

bool ffCacheRead(const FFinstance* instance, void* obj, const char* cacheName, FFCacheMethodCallback callback)
{
    if(instance->config.recache)
        return false;

    FFCacheRead cache;
    bool result;

    FFstrbuf path;
    ffStrbufInitA(&path, 128);
    getCacheFilePath(instance, cacheName, FF_CACHE_EXTENSION_V1, &path);

    cache.position = 0;

    ffStrbufInitA(&cache.data, 256);
    result = ffAppendFileBuffer(path.chars, &cache.data);

    if(result)
        result = callback(obj, &cache, (FFCacheMethodStrbuf) cacheReadStrbuf, (FFCacheMethodData) cacheReadData);

    if(result)
        result = cache.position == cache.data.length;

    if(!result)
        callback(obj, NULL, (FFCacheMethodStrbuf) cacheResetStrbuf, (FFCacheMethodData) cacheResetData);

    ffStrbufDestroy(&cache.data);
    ffStrbufDestroy(&path);

    return result;
}

typedef struct FFCacheWrite
{
    FFstrbuf data;
} FFCacheWrite;

static bool cacheWriteStrbuf(FFCacheWrite* cacheWrite, FFstrbuf* strbuf)
{
    ffStrbufEnsureFree(&cacheWrite->data, strbuf->length);
    memcpy(cacheWrite->data.chars + cacheWrite->data.length, strbuf->chars, strbuf->length +1); //Copy the nullbyte too
    cacheWrite->data.length += strbuf->length + 1;
    return true;
}

static bool cacheWriteData(FFCacheWrite* cacheWrite, size_t dataSize, const void* data)
{
    ffStrbufEnsureFree(&cacheWrite->data, (uint32_t) dataSize);
    memcpy(cacheWrite->data.chars + cacheWrite->data.length, data, dataSize);
    cacheWrite->data.length += (uint32_t) dataSize;
    return true;
}

void ffCacheWrite(const FFinstance* instance, void* obj, const char* cacheName, FFCacheMethodCallback callback)
{
    if(!instance->config.cacheSave)
        return;

    FFCacheWrite cache;

    FFstrbuf path;
    ffStrbufInitA(&path, 128);
    getCacheFilePath(instance, cacheName, FF_CACHE_EXTENSION_V1, &path);

    ffStrbufInitA(&cache.data, 256);
    callback(obj, &cache, (FFCacheMethodStrbuf) cacheWriteStrbuf, (FFCacheMethodData) cacheWriteData);
    ffWriteFileBuffer(path.chars, &cache.data);

    ffStrbufDestroy(&cache.data);
    ffStrbufDestroy(&path);
}

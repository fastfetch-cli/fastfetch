#include "fastfetch.h"
#include "common/caching.h"
#include "common/io.h"
#include "common/printing.h"

#include <stdlib.h>

#define FF_CACHE_VALUE_EXTENSION "ffcv"
#define FF_CACHE_SPLIT_EXTENSION "ffcs"

#define FF_CACHE_EXTENSION_DATA "ffcd"

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
    readCacheFile(instance, "cacheversion", "ffv", &content);

    bool isSameVersion = ffStrbufCompS(&content, FASTFETCH_PROJECT_VERSION) == 0;
    ffStrbufDestroy(&content);
    if(isSameVersion)
        return;

    instance->config.recache = true;

    FFstrbuf version;
    ffStrbufInitA(&version, sizeof(FASTFETCH_PROJECT_VERSION));
    ffStrbufAppendS(&version, FASTFETCH_PROJECT_VERSION);
    writeCacheFile(instance, "cacheversion", "ffv", &version);
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

void ffCachingWriteData(const FFinstance* instance, const char* moduleName, size_t dataSize, const void* data)
{
    FFstrbuf path;
    ffStrbufInitA(&path, 128);
    getCacheFilePath(instance, moduleName, FF_CACHE_EXTENSION_DATA, &path);
    ffWriteFileData(path.chars, dataSize, data);
    ffStrbufDestroy(&path);
}

bool ffCachingReadData(const FFinstance* instance, const char* moduleName, size_t dataSize, void* data)
{
    if(instance->config.recache)
        return false;

    FFstrbuf path;
    ffStrbufInitA(&path, 128);
    getCacheFilePath(instance, moduleName, FF_CACHE_EXTENSION_DATA, &path);
    ssize_t result = ffReadFileData(path.chars, dataSize, data);
    ffStrbufDestroy(&path);
    return result > 0 && (size_t) result == dataSize;
}

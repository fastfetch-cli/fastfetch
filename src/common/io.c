#include "fastfetch.h"

#include <malloc.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>

#define FF_IO_CACHE_VALUE_EXTENSION "ffcv"
#define FF_IO_CACHE_SPLIT_EXTENSION "ffcs"

void ffPrintLogoAndKey(FFinstance* instance, const char* moduleName, uint8_t moduleIndex, const FFstrbuf* customKeyFormat)
{
    ffPrintLogoLine(instance);

    fputs(FASTFETCH_TEXT_MODIFIER_BOLT, stdout);
    ffStrbufWriteTo(&instance->config.color, stdout);

    FF_STRBUF_CREATE(key);

    if(customKeyFormat == NULL || customKeyFormat->length == 0)
    {
        ffStrbufAppendS(&key, moduleName);

        if(moduleIndex > 0)
            ffStrbufAppendF(&key, " %hhu", moduleIndex);
    }
    else
    {
        ffParseFormatString(&key, customKeyFormat, NULL, 1, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_UINT8, &moduleIndex}
        });
    }

    ffStrbufWriteTo(&key, stdout);
    fputs(FASTFETCH_TEXT_MODIFIER_RESET, stdout);
    ffStrbufWriteTo(&instance->config.seperator, stdout);
    ffStrbufDestroy(&key);
}

void ffPrintError(FFinstance* instance, const char* moduleName, uint8_t moduleIndex, const FFstrbuf* customKeyFormat, const FFstrbuf* formatString, uint32_t numFormatArgs, const char* message, ...)
{
    va_list arguments;
    va_start(arguments, message);

    if((formatString == NULL || formatString->length == 0) && instance->config.showErrors)
    {
        ffPrintLogoAndKey(instance, moduleName, moduleIndex, customKeyFormat);
        fputs(FASTFETCH_TEXT_MODIFIER_ERROR, stdout);
        vprintf(message, arguments);
        puts(FASTFETCH_TEXT_MODIFIER_RESET);
    }
    else
    {
        FF_STRBUF_CREATE(error);
        ffStrbufAppendVF(&error, message, arguments);

        // calloc sets all to 0 and FF_FORMAT_ARG_TYPE_NULL also has value 0 so we don't need to explictly set it
        FFformatarg* nullArgs = calloc(numFormatArgs, sizeof(FFformatarg));

        ffPrintFormatString(instance, moduleName, moduleIndex, customKeyFormat, formatString, &error, numFormatArgs, nullArgs);

        free(nullArgs);
        ffStrbufDestroy(&error);
    }

    va_end(arguments);
}

void ffPrintFormatString(FFinstance* instance, const char* moduleName, uint8_t moduleIndex, const FFstrbuf* customKeyFormat, const FFstrbuf* formatString, const FFstrbuf* error, uint32_t numArgs, const FFformatarg* arguments)
{
    FFstrbuf buffer;
    ffStrbufInitA(&buffer, 256);

    ffParseFormatString(&buffer, formatString, error, numArgs, arguments);

    if(buffer.length > 0)
    {
        ffPrintLogoAndKey(instance, moduleName, moduleIndex, customKeyFormat);
        ffStrbufPutTo(&buffer, stdout);
    }

    ffStrbufDestroy(&buffer);
}

static void appendCacheDir(FFinstance* instance, FFstrbuf* buffer)
{
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    static FFstrbuf cacheDir;
    static bool init = false;

    pthread_mutex_lock(&mutex);

    if(init)
    {
        pthread_mutex_unlock(&mutex);
        ffStrbufAppend(buffer, &cacheDir);
        return;
    }

    ffStrbufInitA(&cacheDir, 64);
    ffStrbufAppendS(&cacheDir, getenv("XDG_CACHE_HOME"));

    if(cacheDir.length == 0)
    {
        ffStrbufSetS(&cacheDir, instance->state.passwd->pw_dir);
        ffStrbufAppendS(&cacheDir, "/.cache/");
    }

    mkdir(cacheDir.chars, S_IRWXU | S_IXGRP | S_IRGRP | S_IXOTH | S_IROTH); //I hope everybody has a cache folder but whow knews

    ffStrbufAppendS(&cacheDir, "fastfetch/");
    mkdir(cacheDir.chars, S_IRWXU | S_IRGRP | S_IROTH);

    init = true;

    pthread_mutex_unlock(&mutex);

    ffStrbufAppend(buffer, &cacheDir);
}

void ffGetCacheFilePath(FFinstance* instance, const char* moduleName, const char* extension, FFstrbuf* buffer)
{
    appendCacheDir(instance, buffer);
    ffStrbufAppendS(buffer, moduleName);

    if(extension != NULL)
    {
        ffStrbufAppendC(buffer, '.');
        ffStrbufAppendS(buffer, extension);
    }
}

void ffReadCacheFile(FFinstance* instance, const char* moduleName, const char* extension, FFstrbuf* buffer)
{
    FFstrbuf path;
    ffStrbufInitA(&path, 64);
    ffGetCacheFilePath(instance, moduleName, extension, &path);
    ffAppendFileContent(path.chars, buffer);
    ffStrbufDestroy(&path);
}

void ffWriteCacheFile(FFinstance* instance, const char* moduleName, const char* extension, FFstrbuf* content)
{
    FFstrbuf path;
    ffStrbufInitA(&path, 64);
    ffGetCacheFilePath(instance, moduleName, extension, &path);
    ffWriteFileContent(path.chars, content);
    ffStrbufDestroy(&path);
}

static bool printCachedValue(FFinstance* instance, const char* moduleName, const FFstrbuf* customKeyFormat)
{
    FFstrbuf content;
    ffStrbufInitA(&content, 512);
    ffReadCacheFile(instance, moduleName, FF_IO_CACHE_VALUE_EXTENSION, &content);

    ffStrbufTrimRight(&content, '\0'); //Strbuf always appends a '\0' at the end. We want the last null byte to be at the position of the length

    if(content.length == 0)
        return false;

    uint8_t moduleCounter = 1;

    uint32_t lastIndex = 0;
    while(lastIndex < content.length)
    {
        uint32_t nullByteIndex = ffStrbufFirstIndexAfterC(&content, lastIndex, '\0');
        uint8_t moduleIndex = (moduleCounter == 1 && nullByteIndex == content.length) ? 0 : moduleCounter;
        ffPrintLogoAndKey(instance, moduleName, moduleIndex, customKeyFormat);
        puts(content.chars + lastIndex);
        lastIndex = nullByteIndex + 1;
        ++moduleCounter;
    }

    ffStrbufDestroy(&content);

    return moduleCounter > 1;
}

static bool printCachedFormat(FFinstance* instance, const char* moduleName, const FFstrbuf* customKeyFormat, const FFstrbuf* formatString, uint32_t numArgs)
{
    FFstrbuf content;
    ffStrbufInitA(&content, 512);
    ffReadCacheFile(instance, moduleName, FF_IO_CACHE_SPLIT_EXTENSION, &content);

    ffStrbufTrimRight(&content, '\0'); //Strbuf always appends a '\0' at the end. We want the last null byte to be at the position of the length

    if(content.length == 0)
        return false;

    uint8_t moduleCounter = 1;

    FFformatarg* arguments = calloc(numArgs, sizeof(FFformatarg));
    uint32_t argumentCounter = 0;

    uint32_t lastIndex = 0;
    while(lastIndex < content.length)
    {
        arguments[argumentCounter].type = FF_FORMAT_ARG_TYPE_STRING;
        arguments[argumentCounter].value = &content.chars[lastIndex];
        ++argumentCounter;

        uint32_t nullByteIndex = ffStrbufFirstIndexAfterC(&content, lastIndex, '\0');

        if(argumentCounter == numArgs)
        {
            uint8_t moduleIndex = (moduleCounter == 1 && nullByteIndex == content.length) ? 0 : moduleCounter;
            ffPrintFormatString(instance, moduleName, moduleIndex, customKeyFormat, formatString, NULL, numArgs, arguments);
            ++moduleCounter;
            argumentCounter = 0;
        }

        lastIndex = nullByteIndex + 1;
    }

    free(arguments);
    ffStrbufDestroy(&content);

    return moduleCounter > 1;
}

bool ffPrintFromCache(FFinstance* instance, const char* moduleName, const FFstrbuf* customKeyFormat, const FFstrbuf* formatString, uint32_t numArgs)
{
    if(instance->config.recache)
        return false;

    if(formatString == NULL || formatString->length == 0)
        return printCachedValue(instance, moduleName, customKeyFormat);
    else
        return printCachedFormat(instance, moduleName, customKeyFormat, formatString, numArgs);
}

void ffPrintAndAppendToCache(FFinstance* instance, const char* moduleName, uint8_t moduleIndex, const FFstrbuf* customKeyFormat, FFcache* cache, const FFstrbuf* value, const FFstrbuf* formatString, uint32_t numArgs, const FFformatarg* arguments)
{
    if(formatString == NULL || formatString->length == 0)
    {
        ffPrintLogoAndKey(instance, moduleName, moduleIndex, customKeyFormat);
        ffStrbufPutTo(value, stdout);
    }
    else
    {
        ffPrintFormatString(instance, moduleName, moduleIndex, customKeyFormat, formatString, NULL, numArgs, arguments);
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
        ffStrbufInit(&buffer);
        ffFormatAppendFormatArg(&buffer, &arguments[i]);
        ffStrbufWriteTo(&buffer, cache->split);
        ffStrbufDestroy(&buffer);
        fputc('\0', cache->split);
    }
}

void ffPrintAndSaveToCache(FFinstance* instance, const char* moduleName, const FFstrbuf* customKeyFormat, const FFstrbuf* value, const FFstrbuf* formatString, uint32_t numArgs, const FFformatarg* arguments)
{
    FFcache cache;
    ffCacheOpenWrite(instance, moduleName, &cache);
    ffPrintAndAppendToCache(instance, moduleName, 0, customKeyFormat, &cache, value, formatString, numArgs, arguments);
    ffCacheClose(&cache);
}

void ffCacheValidate(FFinstance* instance)
{
    FFstrbuf path;
    ffStrbufInitA(&path, 64);
    ffGetCacheFilePath(instance, "cacheversion", "ffv", &path);

    FFstrbuf content;
    ffStrbufInit(&content);
    ffAppendFileContent(path.chars, &content);

    if(ffStrbufCompS(&content, FASTFETCH_PROJECT_VERSION) == 0)
    {
        ffStrbufDestroy(&content);
        ffStrbufDestroy(&path);
        return;
    }

    instance->config.recache = true;

    FFstrbuf version;
    ffStrbufInit(&version);
    ffStrbufAppendS(&version, FASTFETCH_PROJECT_VERSION);
    ffWriteFileContent(path.chars, &version);
    ffStrbufDestroy(&version);

    ffStrbufDestroy(&content);
    ffStrbufDestroy(&path);
}

void ffCacheOpenWrite(FFinstance* instance, const char* moduleName, FFcache* cache)
{
    FFstrbuf cacheFileValue;
    ffStrbufInitA(&cacheFileValue, 64);
    ffGetCacheFilePath(instance, moduleName, FF_IO_CACHE_VALUE_EXTENSION, &cacheFileValue);
    cache->value = fopen(cacheFileValue.chars, "w");
    ffStrbufDestroy(&cacheFileValue);

    FFstrbuf cacheFileSplit;
    ffStrbufInitA(&cacheFileSplit, 64);
    ffGetCacheFilePath(instance, moduleName, FF_IO_CACHE_SPLIT_EXTENSION, &cacheFileSplit);
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

bool ffParsePropFile(const char* fileName, const char* regex, char* buffer)
{
    buffer[0] = '\0'; //If an error occures, this is the indicator

    char* line = NULL;
    size_t len = 0;

    FILE* file = fopen(fileName, "r");
    if(file == NULL)
        return false; // handle errors in higher functions

    while (getline(&line, &len, file) != -1)
    {
        if (sscanf(line, regex, buffer) > 0)
            break;
    }

    fclose(file);
    if(line != NULL)
        free(line);

    return true;
}

bool ffParsePropFileHome(FFinstance* instance, const char* relativeFile, const char* regex, char* buffer)
{
    FFstrbuf absolutePath;
    ffStrbufInitA(&absolutePath, 64);
    ffStrbufAppendS(&absolutePath, instance->state.passwd->pw_dir);
    ffStrbufAppendC(&absolutePath, '/');
    ffStrbufAppendS(&absolutePath, relativeFile);

    bool result = ffParsePropFile(absolutePath.chars, regex, buffer);

    ffStrbufDestroy(&absolutePath);

    return result;
}

bool ffParsePropFileConfig(FFinstance* instance, const char* relativeFile, const char* regex, char* buffer)
{
    bool foundAFile = false;

    for(uint32_t i = 0; i < instance->state.configDirs.length; i++)
    {
        FFstrbuf* baseDir = (FFstrbuf*) ffListGet(&instance->state.configDirs, i);
        uint32_t baseDirLength = baseDir->length;

        if(*relativeFile != '/')
            ffStrbufAppendC(baseDir, '/');

        ffStrbufAppendS(baseDir, relativeFile);

        if(ffParsePropFile(baseDir->chars, regex, buffer))
            foundAFile = true;

        ffStrbufSubstrBefore(baseDir, baseDirLength);

        if(*buffer != '\0')
            return foundAFile;
    }

    return foundAFile;
}

bool ffWriteFDContent(int fd, const FFstrbuf* content)
{
    return write(fd, content->chars, content->length) != -1;
}

void ffWriteFileContent(const char* fileName, const FFstrbuf* content)
{
    int fd = open(fileName, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(fd == -1)
        return;

    ffWriteFDContent(fd, content);

    close(fd);
}

void ffAppendFDContent(int fd, FFstrbuf* buffer)
{
    ssize_t readed;
    while((readed = read(fd, buffer->chars + buffer->length, buffer->allocated - buffer->length)) == (buffer->allocated - buffer->length))
    {
        buffer->length += (uint32_t) readed;
        ffStrbufEnsureCapacity(buffer, buffer->allocated * 2);
    }

    if(readed >= 0)
        buffer->length += (uint32_t) readed;

    ffStrbufTrimRight(buffer, '\n');
    ffStrbufTrimRight(buffer, ' ');
}

void ffAppendFileContent(const char* fileName, FFstrbuf* buffer)
{
    int fd = open(fileName, O_RDONLY);
    if(fd == -1)
        return;

    ffAppendFDContent(fd, buffer);

    close(fd);
}

void ffGetFileContent(const char* fileName, FFstrbuf* buffer)
{
    ffStrbufClear(buffer);
    ffAppendFileContent(fileName, buffer);
}

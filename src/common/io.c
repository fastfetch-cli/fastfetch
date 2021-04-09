#include "fastfetch.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

void ffPrintKey(FFinstance* instance, FFstrbuf* customKey, const char* defKey)
{
    fputs(FASTFETCH_TEXT_MODIFIER_BOLT, stdout);
    ffStrbufWriteTo(&instance->config.color, stdout);
    if(customKey == NULL || customKey->length == 0)
        fputs(defKey, stdout);
    else
        ffStrbufWriteTo(customKey, stdout);
    fputs(FASTFETCH_TEXT_MODIFIER_RESET, stdout);
    ffStrbufWriteTo(&instance->config.seperator, stdout);
}

void ffPrintLogoAndKey(FFinstance* instance, FFstrbuf* customKey, const char* defKey)
{
    ffPrintLogoLine(instance);
    ffPrintKey(instance, customKey, defKey);
}

void ffPrintError(FFinstance* instance, FFstrbuf* customKey, const char* defKey, const char* message, ...)
{
    if(!instance->config.showErrors)
        return;

    ffPrintLogoAndKey(instance, customKey, defKey);

    va_list arguments;
    va_start(arguments, message);

    printf(FASTFETCH_TEXT_MODIFIER_ERROR);
    vprintf(message, arguments);
    puts(FASTFETCH_TEXT_MODIFIER_RESET);

    va_end(arguments);
}

void ffParsePropFile(const char* fileName, const char* regex, char* buffer)
{
    buffer[0] = '\0'; //If an error occures, this is the indicator

    char* line = NULL;
    size_t len = 0;

    FILE* file = fopen(fileName, "r");
    if(file == NULL)
        return; // handle errors in higher functions

    while (getline(&line, &len, file) != -1)
    {
        if (sscanf(line, regex, buffer) > 0)
            break;
    }

    fclose(file);
    if(line != NULL)
        free(line);
}

void ffParsePropFileHome(FFinstance* instance, const char* relativeFile, const char* regex, char* buffer)
{
    FFstrbuf absolutePath;
    ffStrbufInitA(&absolutePath, 64);
    ffStrbufAppendS(&absolutePath, instance->state.passwd->pw_dir);
    ffStrbufAppendC(&absolutePath, '/');
    ffStrbufAppendS(&absolutePath, relativeFile);

    ffParsePropFile(absolutePath.chars, regex, buffer);

    ffStrbufDestroy(&absolutePath);
}

void ffAppendFileContent(const char* fileName, FFstrbuf* buffer)
{
    int fd = open(fileName, O_RDONLY);
    if(fd == -1)
        return;

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

    close(fd);
}

void ffGetFileContent(const char* fileName, FFstrbuf* buffer)
{
    ffStrbufClear(buffer);
    ffAppendFileContent(fileName, buffer);
}

static const FFstrbuf* getCacheDir(FFinstance* instance)
{
    static FFstrbuf cacheDir;
    static bool init = false;
    if(init)
        return &cacheDir;

    ffStrbufInitAS(&cacheDir, 64, getenv("XDG_CACHE_HOME"));

    if(cacheDir.length == 0)
    {
        ffStrbufSetS(&cacheDir, instance->state.passwd->pw_dir);
        ffStrbufAppendS(&cacheDir, "/.cache/");
    }

    mkdir(cacheDir.chars, S_IRWXU | S_IXGRP | S_IRGRP | S_IXOTH | S_IROTH); //I hope everybody has a cache folder but whow knews

    ffStrbufAppendS(&cacheDir, "fastfetch/");
    mkdir(cacheDir.chars, S_IRWXU | S_IRGRP | S_IROTH);

    init = true;
    return &cacheDir;
}

bool ffPrintCachedValue(FFinstance* instance, FFstrbuf* customKey, const char* defKey)
{
    if(instance->config.recache)
        return false;

    FFstrbuf cacheFile;
    ffStrbufInitA(&cacheFile, 128);
    ffStrbufAppend(&cacheFile, getCacheDir(instance));
    ffStrbufAppendS(&cacheFile, defKey);

    FFstrbuf value;
    ffStrbufInitA(&value, 128);
    ffGetFileContent(cacheFile.chars, &value);

    ffStrbufDestroy(&cacheFile);

    if(value.length == 0)
    {
        ffStrbufDestroy(&value);
        return false;
    }

    ffPrintLogoAndKey(instance, customKey, defKey);
    ffStrbufPutTo(&value, stdout);
    ffStrbufDestroy(&value);
    return true;
}

void ffPrintAndSaveCachedValue(FFinstance* instance, FFstrbuf* customKey, const char* defKey, FFstrbuf* value)
{
    if(value->length == 0)
        return;

    ffPrintLogoAndKey(instance, customKey, defKey);
    ffStrbufPutTo(value, stdout);

    if(!instance->config.cacheSave)
        return;

    FFstrbuf cacheFile;
    ffStrbufInit(&cacheFile);
    ffStrbufAppend(&cacheFile, getCacheDir(instance));
    ffStrbufAppendS(&cacheFile, defKey);

    int fd = open(cacheFile.chars, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(fd == -1)
        return;

    bool failed = write(fd, value->chars, value->length) != value->length;

    close(fd);

    if(failed)
        unlink(cacheFile.chars);

    ffStrbufDestroy(&cacheFile);
}

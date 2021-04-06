#include "fastfetch.h"

#include <malloc.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdarg.h>

void ffInitState(FFstate* state)
{
    state->current_row = 0;
    state->passwd = getpwuid(getuid());
    uname(&state->utsname);
    sysinfo(&state->sysinfo);
}

void ffDefaultConfig(FFconfig* config)
{
    ffStrbufInit(&config->color);
    config->logo_spacing = 4;
    ffStrbufInitS(&config->seperator, ": ");
    config->offsetx = 0;
    config->titleLength = 20; // This is overwritten by ffPrintTitle
    config->colorLogo = true;
    config->showErrors = false;
    config->recache = false;
    config->cacheSave = true;
    config->printRemainingLogo = true;

    //Since most of these properties are unlikely to be used at once, give them minimal heap space (the \0 character)
    ffStrbufInitA(&config->osFormat, 1);
    ffStrbufInitA(&config->osKey, 1);
    ffStrbufInitA(&config->hostFormat, 1);
    ffStrbufInitA(&config->hostKey, 1);
    ffStrbufInitA(&config->kernelFormat, 1);
    ffStrbufInitA(&config->kernelKey, 1);
    ffStrbufInitA(&config->uptimeFormat, 1);
    ffStrbufInitA(&config->uptimeKey, 1);
    ffStrbufInitA(&config->packagesFormat, 1);
    ffStrbufInitA(&config->packagesKey, 1);
    ffStrbufInitA(&config->shellFormat, 1);
    ffStrbufInitA(&config->shellKey, 1);
    ffStrbufInitA(&config->resolutionFormat, 1);
    ffStrbufInitA(&config->resolutionKey, 1);
    ffStrbufInitA(&config->deFormat, 1);
    ffStrbufInitA(&config->deKey, 1);
    ffStrbufInitA(&config->wmFormat, 1);
    ffStrbufInitA(&config->wmKey, 1);
    ffStrbufInitA(&config->wmThemeFormat, 1);
    ffStrbufInitA(&config->wmThemeKey, 1);
    ffStrbufInitA(&config->themeFormat, 1);
    ffStrbufInitA(&config->themeKey, 1);
    ffStrbufInitA(&config->iconsFormat, 1);
    ffStrbufInitA(&config->iconsKey, 1);
    ffStrbufInitA(&config->fontFormat, 1);
    ffStrbufInitA(&config->fontKey, 1);
    ffStrbufInitA(&config->terminalFormat, 1);
    ffStrbufInitA(&config->terminalKey, 1);
    ffStrbufInitA(&config->termFontFormat, 1);
    ffStrbufInitA(&config->termFontKey, 1);
    ffStrbufInitA(&config->cpuFormat, 1);
    ffStrbufInitA(&config->cpuKey, 1);
    ffStrbufInitA(&config->gpuFormat, 1);
    ffStrbufInitA(&config->gpuKey, 1);
    ffStrbufInitA(&config->memoryFormat, 1);
    ffStrbufInitA(&config->memoryKey, 1);
    ffStrbufInitA(&config->diskFormat, 1);
    ffStrbufInitA(&config->diskKey, 1);
    ffStrbufInitA(&config->batteryFormat, 1);
    ffStrbufInitA(&config->batteryKey, 1);
    ffStrbufInitA(&config->localeFormat, 1);
    ffStrbufInitA(&config->localeKey, 1);

    ffStrbufInitA(&config->libPCI, 1);
    ffStrbufInitA(&config->libX11, 1);
    ffStrbufInitA(&config->libXrandr, 1);
    ffStrbufInitA(&config->libDConf, 1);

    ffStrbufInitA(&config->diskFolders, 1);
}

static void ffCleanup(FFinstance* instance)
{
    // Place for cleaning up
    // I dont destroy the global strbufs, because the OS is typically faster doing it after the program terminates
    // This eliminates one function call per strbuf + setting things like length to 0
}

void ffFinish(FFinstance* instance)
{
    if(instance->config.printRemainingLogo)
        ffPrintRemainingLogo(instance);

    ffCleanup(instance);
}

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
        buffer->length += readed;
        ffStrbufEnsureCapacity(buffer, buffer->allocated * 2);
    }

    if(readed >= 0)
        buffer->length += readed;

    ffStrbufTrimRight(buffer, '\n');
    ffStrbufTrimRight(buffer, ' ');

    close(fd);
}

void ffGetFileContent(const char* fileName, FFstrbuf* buffer)
{
    ffStrbufClear(buffer);
    ffAppendFileContent(fileName, buffer);
}

void ffParseFormatStringV(FFstrbuf* buffer, FFstrbuf* formatstr, uint32_t numArgs, va_list argp)
{
    FFformatarg arguments[numArgs];
    for(uint32_t i = 0; i < numArgs; i++)
        arguments[i] = (FFformatarg) va_arg(argp, FFformatarg);

    uint32_t argCounter = 1; //First arg is 1 in fomat string

    for(uint32_t i = 0; i < formatstr->length; ++i)
    {
        if(formatstr->chars[i] != '{')
        {
            ffStrbufAppendC(buffer, formatstr->chars[i]);
            continue;
        }

        if(i == formatstr->length - 1)
        {
            ffStrbufAppendC(buffer, '{');
            continue; //This will always stop the loop
        }

        ++i;

        if(formatstr->chars[i] == '{')
        {
            ffStrbufAppendC(buffer, '{');
            continue;
        }

        uint32_t argIndex;

        if(formatstr->chars[i] == '}')
        {
            if(argCounter > numArgs)
            {
                ffStrbufAppendS(buffer, "{}");
                continue;
            }

            argIndex = argCounter++;
        }
        else
        {
            FFstrbuf argnumstr;
            ffStrbufInit(&argnumstr);

            while(formatstr->chars[i] != '}' && i < formatstr->length)
                ffStrbufAppendC(&argnumstr, formatstr->chars[i++]);

            if(
                argnumstr.length == 0 ||
                ffStrbufGetC(&argnumstr, 0) == '-' ||
                sscanf(argnumstr.chars, "%u", &argIndex) != 1 ||
                argIndex > numArgs
            ) {
                ffStrbufAppendC(buffer, '{');
                ffStrbufAppend(buffer, &argnumstr);
                if(formatstr->chars[i] == '}') // We dont have a closing { when ending because whole format string is over
                    ffStrbufAppendC(buffer, '}');
                ffStrbufDestroy(&argnumstr);
                continue;
            }

            ffStrbufDestroy(&argnumstr);
        }

        if(argIndex == 0)
            argIndex = 1;

        FFformatarg arg = arguments[argIndex - 1];

        if(arg.type == FF_FORMAT_ARG_TYPE_INT)
            ffStrbufAppendF(buffer, "%i", *(int*)arg.value);
        else if(arg.type == FF_FORMAT_ARG_TYPE_UINT)
            ffStrbufAppendF(buffer, "%u", *(uint32_t*)arg.value);
        else if(arg.type == FF_FORMAT_ARG_TYPE_UINT8)
            ffStrbufAppendF(buffer, "%u", *(uint8_t*)arg.value);
        else if(arg.type == FF_FORMAT_ARG_TYPE_STRING)
            ffStrbufAppendF(buffer, "%s", (const char*)arg.value);
        else if(arg.type == FF_FORMAT_ARG_TYPE_STRBUF)
            ffStrbufAppend(buffer, (FFstrbuf*)arg.value);
        else if(arg.type == FF_FORMAT_ARG_TYPE_DOUBLE)
            ffStrbufAppendF(buffer, "%g", *(double*)arg.value);
        else
        {
            fprintf(stderr, "Error: format string \"%s\" argument is not implemented\n", formatstr->chars);
            ffStrbufDestroy(buffer);
            exit(806);
        }
    }

    ffStrbufTrimRight(buffer, ' ');
}

void ffParseFormatString(FFstrbuf* buffer, FFstrbuf* formatstr, uint32_t numArgs, ...)
{
    va_list argp;
    va_start(argp, numArgs);
    ffParseFormatStringV(buffer, formatstr, numArgs, argp);
    va_end(argp);
}

void ffPrintFormatString(FFinstance* instance, FFstrbuf* customKey, const char* defKey, FFstrbuf* formatstr, uint32_t numArgs, ...)
{
    FFstrbuf buffer;
    ffStrbufInitA(&buffer, 256);

    va_list argp;
    va_start(argp, numArgs);

    ffParseFormatStringV(&buffer, formatstr, numArgs, argp);

    va_end(argp);

    if(buffer.length > 0)
    {
        ffPrintLogoAndKey(instance, customKey, defKey);
        ffStrbufPutTo(&buffer, stdout);
    }

    ffStrbufDestroy(&buffer);
}

void ffFormatGtkPretty(FFstrbuf* buffer, FFstrbuf* gtk2, FFstrbuf* gtk3, FFstrbuf* gtk4)
{
    if(gtk2->length > 0 && gtk3->length > 0 && gtk4->length > 0)
    {
        if((ffStrbufIgnCaseComp(gtk2, gtk3) == 0) && (ffStrbufIgnCaseComp(gtk2, gtk4) == 0))
        {
            ffStrbufAppend(buffer, gtk4);
            ffStrbufAppendS(buffer, " [GTK2/3/4]");
        }
        else if(ffStrbufIgnCaseComp(gtk2, gtk3) == 0)
        {
            ffStrbufAppend(buffer, gtk3);
            ffStrbufAppendS(buffer, " [GTK2/3], ");
            ffStrbufAppend(buffer, gtk4);
            ffStrbufAppendS(buffer, " [GTK4]");
        }
        else if(ffStrbufIgnCaseComp(gtk3, gtk4) == 0)
        {
            ffStrbufAppend(buffer, gtk2);
            ffStrbufAppendS(buffer, " [GTK2], ");
            ffStrbufAppend(buffer, gtk4);
            ffStrbufAppendS(buffer, " [GTK3/4]");
        }
        else
        {
            ffStrbufAppend(buffer, gtk2);
            ffStrbufAppendS(buffer, " [GTK2], ");
            ffStrbufAppend(buffer, gtk3);
            ffStrbufAppendS(buffer, " [GTK3], ");
            ffStrbufAppend(buffer, gtk4);
            ffStrbufAppendS(buffer, " [GTK4]");
        }
    }
    else if(gtk2->length > 0 && gtk3->length > 0)
    {
        if(ffStrbufIgnCaseComp(gtk2, gtk3) == 0)
        {
            ffStrbufAppend(buffer, gtk3);
            ffStrbufAppendS(buffer, " [GTK2/3]");
        }
        else
        {
            ffStrbufAppend(buffer, gtk2);
            ffStrbufAppendS(buffer, " [GTK2], ");
            ffStrbufAppend(buffer, gtk3);
            ffStrbufAppendS(buffer, " [GTK3]");
        }
    }
    else if(gtk3->length > 0 && gtk4->length > 0)
    {
        if(ffStrbufIgnCaseComp(gtk3, gtk4) == 0)
        {
            ffStrbufAppend(buffer, gtk4);
            ffStrbufAppendS(buffer, " [GTK3/4]");
        }
        else
        {
            ffStrbufAppend(buffer, gtk3);
            ffStrbufAppendS(buffer, " [GTK3], ");
            ffStrbufAppend(buffer, gtk4);
            ffStrbufAppendS(buffer, " [GTK4]");
        }
    }
    else if(gtk2->length > 0)
    {
        ffStrbufAppend(buffer, gtk2);
        ffStrbufAppendS(buffer, " [GTK2]");
    }
    else if(gtk3->length > 0)
    {
        ffStrbufAppend(buffer, gtk3);
        ffStrbufAppendS(buffer, " [GTK3]");
    }
    else if(gtk4->length > 0)
    {
        ffStrbufAppend(buffer, gtk4);
        ffStrbufAppendS(buffer, " [GTK4]");
    }
}

void ffParseFont(const char* font, FFstrbuf* name, double* size)
{
    ffStrbufEnsureCapacity(name, 32);

    int scanned = sscanf(font, "%[^,], %lf", name->chars, size);

    ffStrbufRecalculateLength(name);

    if(scanned < 2)
        *size = 0.0;

    if(scanned == 0)
        ffStrbufSetS(name, font);
}

void ffFontPretty(FFstrbuf* buffer, const FFstrbuf* name, double size)
{
    ffStrbufAppend(buffer, name);

    if(size > 0)
    {
        ffStrbufAppendS(buffer, " (");
        ffStrbufAppendF(buffer, "%g", size);
        ffStrbufAppendS(buffer, "pt)");
    }
}

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
    
    ffStrbufInit(&state->terminal.value);
    state->terminal.error = NULL;
    state->terminal.calculated = false;
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

    ffStrbufInit(&config->osFormat);
    ffStrbufInit(&config->hostFormat);
    ffStrbufInit(&config->kernelFormat);
    ffStrbufInit(&config->uptimeFormat);
    ffStrbufInit(&config->packagesFormat);
    ffStrbufInit(&config->shellFormat);
    ffStrbufInit(&config->resolutionFormat);
    ffStrbufInit(&config->deFormat);
    ffStrbufInit(&config->wmFormat);
    ffStrbufInit(&config->themeFormat);
    ffStrbufInit(&config->iconsFormat);
    ffStrbufInit(&config->fontFormat);
    ffStrbufInit(&config->terminalFormat);
    ffStrbufInit(&config->termFontFormat);
    ffStrbufInit(&config->cpuFormat);
    ffStrbufInit(&config->gpuFormat);
    ffStrbufInit(&config->memoryFormat);
    ffStrbufInit(&config->diskFormat);
    ffStrbufInit(&config->batteryFormat);
    ffStrbufInit(&config->localeFormat);

    ffStrbufInitA(&config->libPCI, 256);
    ffStrbufInitA(&config->libX11, 256);
    ffStrbufInitA(&config->libXrandr, 256);
}

static void ffCleanup(FFinstance* instance)
{
    ffStrbufDestroy(&instance->config.color);
    ffStrbufDestroy(&instance->config.seperator);

    ffStrbufDestroy(&instance->config.osFormat);
    ffStrbufDestroy(&instance->config.hostFormat);
    ffStrbufDestroy(&instance->config.kernelFormat);
    ffStrbufDestroy(&instance->config.uptimeFormat);
    ffStrbufDestroy(&instance->config.packagesFormat);
    ffStrbufDestroy(&instance->config.shellFormat);
    ffStrbufDestroy(&instance->config.resolutionFormat);
    ffStrbufDestroy(&instance->config.deFormat);
    ffStrbufDestroy(&instance->config.wmFormat);
    ffStrbufDestroy(&instance->config.themeFormat);
    ffStrbufDestroy(&instance->config.iconsFormat);
    ffStrbufDestroy(&instance->config.fontFormat);
    ffStrbufDestroy(&instance->config.terminalFormat);
    ffStrbufDestroy(&instance->config.termFontFormat);
    ffStrbufDestroy(&instance->config.cpuFormat);
    ffStrbufDestroy(&instance->config.gpuFormat);
    ffStrbufDestroy(&instance->config.memoryFormat);
    ffStrbufDestroy(&instance->config.diskFormat);
    ffStrbufDestroy(&instance->config.batteryFormat);
    ffStrbufDestroy(&instance->config.localeFormat);

    ffStrbufDestroy(&instance->config.libPCI);
    ffStrbufDestroy(&instance->config.libX11);
    ffStrbufDestroy(&instance->config.libXrandr);

    ffStrbufDestroy(&instance->state.terminal.value);
}

void ffFinish(FFinstance* instance)
{
    if(instance->config.printRemainingLogo)
        ffPrintRemainingLogo(instance);

    ffCleanup(instance);
}

void ffPrintKey(FFconfig* config, const char* key)
{
    printf(FASTFETCH_TEXT_MODIFIER_BOLT"%s%s"FASTFETCH_TEXT_MODIFIER_RESET"%s", config->color.chars, key, config->seperator.chars);
}

void ffPrintLogoAndKey(FFinstance* instance, const char* key)
{
    ffPrintLogoLine(instance);
    ffPrintKey(&instance->config, key);
}

void ffParsePropFile(const char* fileName, const char* regex, char* buffer)
{
    buffer[0] = '\0'; //If an error occures, this is the indicator

    char* line = NULL;
    size_t len;

    FILE* file = fopen(fileName, "r");
    if(file == NULL)
        return; // handle errors in higher functions

    while (getline(&line, &len, file) != -1)
    {
        if (sscanf(line, regex, buffer) > 0)
            break;
    }

    fclose(file);
    free(line);
}

void ffParsePropFileHome(FFinstance* instance, const char* relativeFile, const char* regex, char* buffer)
{
    char absolutePath[512];
    strcpy(absolutePath, instance->state.passwd->pw_dir);
    strcat(absolutePath, "/");
    strcat(absolutePath, relativeFile);

    ffParsePropFile(absolutePath, regex, buffer);
}

static inline bool strSet(const char* str)
{
    return str != NULL && str[0] != '\0';
}

void ffFormatGtkPretty(FFstrbuf* buffer, const char* gtk2, const char* gtk3, const char* gtk4)
{
    if(strSet(gtk2) && strSet(gtk3) && strSet(gtk4))
    {
        if((strcmp(gtk2, gtk3) == 0) && (strcmp(gtk2, gtk4) == 0))
            ffStrbufSetF(buffer, "%s [GTK2/3/4]", gtk2);
        else if(strcmp(gtk2, gtk3) == 0)
            ffStrbufSetF(buffer, "%s [GTK2/3], %s [GTK4]", gtk2, gtk4);
        else if(strcmp(gtk3, gtk4) == 0)
            ffStrbufSetF(buffer, "%s [GTK2], %s [GTK3/4]", gtk2, gtk3);
        else
            ffStrbufSetF(buffer, "%s [GTK2], %s [GTK3], %s [GTK4]", gtk2, gtk3, gtk4);
    }
    else if(strSet(gtk2) && strSet(gtk3))
    {
        if(strcmp(gtk2, gtk3) == 0)
            ffStrbufSetF(buffer, "%s [GTK2/3]", gtk2);
        else
            ffStrbufSetF(buffer, "%s [GTK2], %s [GTK3]", gtk2, gtk3);
    }
    else if(strSet(gtk3) && strSet(gtk4))
    {
        if(strcmp(gtk3, gtk4) == 0)
            ffStrbufSetF(buffer, "%s [GTK3/4]", gtk3);
        else
            ffStrbufSetF(buffer, "%s [GTK3], %s [GTK4]", gtk3, gtk4);
    }
    else if(strSet(gtk2))
    {
        ffStrbufSetF(buffer, "%s [GTK2]", gtk2);
    }
    else if(strSet(gtk3))
    {
        ffStrbufSetF(buffer, "%s [GTK3]", gtk3);
    }
    else if(strSet(gtk4))
    {
        ffStrbufSetF(buffer, "%s [GTK4]", gtk4);
    }
}

void ffParseFont(char* font, char* buffer)
{
    char name[64];
    char size[32];
    int scanned = sscanf(font, "%[^,], %[^,]", name, size);
    
    if(scanned == 0)
        strcpy(buffer, font);
    else if(scanned == 1)
        strcpy(buffer, name);
    else
        sprintf(buffer, "%s (%spt)", name, size);
}

void ffPrintError(FFinstance* instance, const char* key, const char* message, ...)
{
    if(!instance->config.showErrors)
        return;

    ffPrintLogoAndKey(instance, key);

    va_list arguments;
    va_start(arguments, message);

    printf(FASTFETCH_TEXT_MODIFIER_ERROR);
    vprintf(message, arguments);
    puts(FASTFETCH_TEXT_MODIFIER_RESET);

    va_end(arguments);
}

void ffTrimTrailingWhitespace(char* buffer)
{
    uint32_t end = 0;

    for(uint32_t i = 0; buffer[i] != '\0'; i++)
    {
        if(buffer[i] != ' ')
            end = i;
    }

    if(buffer[end + 1] == ' ')
        buffer[end + 1] = '\0';
}

void ffGetFileContent(const char* fileName, FFstrbuf* buffer)
{
    int fd = open(fileName, O_RDONLY);
    if(fd == -1)
        return;

    ssize_t readed;
    while((readed = read(fd, buffer->chars, buffer->allocated)) == buffer->allocated)
    {
        ffStrbufEnsureCapacity(buffer, buffer->allocated * 2);
        lseek(fd, 0, SEEK_SET);
    }

    if(readed >= 0)
        buffer->length = readed;

    ffStrbufTrimRight(buffer, '\n');
    ffStrbufTrimRight(buffer, ' ');

    close(fd);
}

static const FFstrbuf* getCacheDir(FFinstance* instance)
{
    static FFstrbuf cacheDir;
    static bool init = false;
    if(init)
        return &cacheDir;

    ffStrbufInitS(&cacheDir, getenv("XDG_CACHE_HOME"));

    if(ffStrbufIsEmpty(&cacheDir))
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

bool ffPrintCachedValue(FFinstance* instance, const char* key)
{
    if(instance->config.recache)
        return false;

    FFstrbuf cacheFile;
    ffStrbufInitA(&cacheFile, 128);
    ffStrbufAppend(&cacheFile, getCacheDir(instance));
    ffStrbufAppendS(&cacheFile, key);

    FF_STRBUF_CREATE(value);
    ffGetFileContent(cacheFile.chars, &value);

    ffStrbufDestroy(&cacheFile);

    if(ffStrbufIsEmpty(&value))
    {
        ffStrbufDestroy(&value);
        return false;
    }

    ffPrintLogoAndKey(instance, key);
    ffStrbufWriteTo(&value, stdout);
    putchar('\n');
    ffStrbufDestroy(&value);
    return true;
}

void ffPrintAndSaveCachedValue(FFinstance* instance, const char* key, FFstrbuf* value)
{
    ffPrintLogoAndKey(instance, key);
    ffStrbufWriteTo(value, stdout);
    putchar('\n');

    if(!instance->config.cacheSave)
        return;

    FFstrbuf cacheFile;
    ffStrbufInit(&cacheFile);
    ffStrbufAppend(&cacheFile, getCacheDir(instance));
    ffStrbufAppendS(&cacheFile, key);

    int fd = open(cacheFile.chars, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(fd == -1)
        return;

    bool failed = write(fd, value->chars, value->length) != value->length;

    close(fd);

    if(failed)
        unlink(cacheFile.chars);

    ffStrbufDestroy(&cacheFile);
}

void ffParseFormatString(FFstrbuf* buffer, FFstrbuf* formatstr, uint32_t numArgs, ...)
{
    va_list argp;
    va_start(argp, numArgs);

    FFformatarg arguments[numArgs];

    for(uint32_t i = 0; i < numArgs; i++)
        arguments[i] = (FFformatarg) va_arg(argp, FFformatarg);

    va_end(argp);

    uint32_t argCounter = 1; //First arg is 1 in fomat string

    for(uint32_t i = 0; i < formatstr->length; ++i)
    {
        if(formatstr->chars[i] != '{')
        {
            ffStrbufAppendC(buffer, formatstr->chars[i]);
            continue;
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
            argIndex = argCounter++;
        }
        else
        {
            FFstrbuf argnumstr;
            ffStrbufInit(&argnumstr);
            
            while(formatstr->chars[i] != '}' && formatstr->chars[i] != '\0')
                ffStrbufAppendC(&argnumstr, formatstr->chars[i++]);

            if(
                ffStrbufGetC(&argnumstr, 0) == '-' ||
                sscanf(argnumstr.chars, "%u", &argIndex) != 1 ||
                argIndex > numArgs
            ) {
                ffStrbufAppendC(buffer, '{');
                ffStrbufAppend(buffer, &argnumstr);
                if(formatstr->chars[i] != '\0')
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
            ffStrbufAppendF(buffer, "%.9g", *(double*)arg.value);
        else
        {
            fprintf(stderr, "Error: format string \"%s\" argument is not implemented\n", formatstr->chars);
            ffStrbufDestroy(buffer);
            exit(806);
        }
    }
}
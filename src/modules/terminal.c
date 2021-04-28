#include "fastfetch.h"

#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define FF_TERMINAL_MODULE_NAME "Terminal"
#define FF_TERMINAL_NUM_FORMAT_ARGS 3

static void getTerminalName(FFinstance* instance, const char* pid, FFTerminalResult* result)
{
    char statFile[234];
    sprintf(statFile, "/proc/%s/stat", pid);

    FILE* stat = fopen(statFile, "r");
    if(stat == NULL)
    {
        ffStrbufSetF(&result->error, "fopen(\"%s\", \"r\") == NULL", statFile);
        return;
    }

    char name[256];
    char ppid[256];
    if(fscanf(stat, "%*s (%[^)])%*s%s", name, ppid) != 2)
    {
        ffStrbufSetS(&result->error, "fscanf(stat, \"%*s (%[^)])%*s%s\", name, ppid) != 2");
        return;
    }

    fclose(stat);

    if (
        strcasecmp(name, "bash")   == 0 ||
        strcasecmp(name, "sh")     == 0 ||
        strcasecmp(name, "zsh")    == 0 ||
        strcasecmp(name, "ksh")    == 0 ||
        strcasecmp(name, "fish")   == 0 ||
        strcasecmp(name, "sudo")   == 0 ||
        strcasecmp(name, "su")     == 0 ||
        strcasecmp(name, "doas")   == 0 ||
        strcasecmp(name, "strace") == 0 )
    {
        getTerminalName(instance, ppid, result);
        return;
    }

    char cmdlineFile[234];
    sprintf(cmdlineFile, "/proc/%s/cmdline", pid);

    ffGetFileContent(cmdlineFile, &result->exeName);
    ffStrbufSubstrBeforeFirstC(&result->exeName, '\0');
    ffStrbufSubstrAfterLastC(&result->exeName, '/');

    ffStrbufSetS(&result->processName, name);
}

const FFTerminalResult* ffDetectTerminal(FFinstance* instance)
{
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    static FFTerminalResult result;
    static bool init = false;
    pthread_mutex_lock(&mutex);
    if(init)
    {
        pthread_mutex_unlock(&mutex);
        return &result;
    }
    init = true;

    ffStrbufInit(&result.exeName);
    ffStrbufInit(&result.processName);
    ffStrbufInit(&result.error);

    char ppid[256];
    sprintf(ppid, "%i", getppid());

    getTerminalName(instance, ppid, &result);

    pthread_mutex_unlock(&mutex);

    return &result;
}

void ffPrintTerminal(FFinstance* instance)
{
    const FFTerminalResult* result = ffDetectTerminal(instance);

    if(result->error.length > 0)
    {
        ffPrintError(instance, FF_TERMINAL_MODULE_NAME, 0, &instance->config.terminalKey, &instance->config.terminalFormat, FF_TERMINAL_NUM_FORMAT_ARGS, result->error.chars);
        return;
    }

    if(result->exeName.length == 0 && result->processName.length == 0)
    {
        ffPrintError(instance, FF_TERMINAL_MODULE_NAME, 0, &instance->config.terminalKey, &instance->config.terminalFormat, FF_TERMINAL_NUM_FORMAT_ARGS, "Terminal names not set");
        return;
    }

    const FFstrbuf* name;

    if(ffStrbufStartsWith(&result->exeName, &result->processName))
        name = &result->exeName;
    else
        name = &result->processName;

    if(instance->config.terminalFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_TERMINAL_MODULE_NAME, 0, &instance->config.terminalKey);
        ffStrbufPutTo(name, stdout);
    }
    else
    {
        ffPrintFormatString(instance, FF_TERMINAL_MODULE_NAME, 0, &instance->config.terminalKey, &instance->config.terminalFormat, NULL, FF_TERMINAL_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->exeName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->processName},
            {FF_FORMAT_ARG_TYPE_STRBUF, name}
        });
    }
}

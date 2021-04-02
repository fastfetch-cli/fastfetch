#include "fastfetch.h"

static void getTerminalName(FFinstance* instance, const char* pid, FFstrbuf* exeName, FFstrbuf* processName, FFstrbuf* error)
{
    char statFile[234];
    sprintf(statFile, "/proc/%s/stat", pid);

    FILE* stat = fopen(statFile, "r");
    if(stat == NULL)
    {
        ffStrbufSetF(error, "fopen(\"%s\", \"r\") == NULL", statFile);
        return;
    }

    char name[256];
    char ppid[256];
    if(fscanf(stat, "%*s (%[^)])%*s%s", name, ppid) != 2)
    {
        ffStrbufSetS(error, "fscanf(stat, \"%*s (%[^)])%*s%s\", name, ppid) != 2");
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
        getTerminalName(instance, ppid, exeName, processName, error);
        return;
    }

    char cmdlineFile[234];
    sprintf(cmdlineFile, "/proc/%s/cmdline", pid);

    ffGetFileContent(cmdlineFile, exeName);
    ffStrbufSubstrBeforeFirstC(exeName, '\0');
    ffStrbufSubstrAfterLastC(exeName, '/');

    ffStrbufSetS(processName, name);
}

void ffGetTerminal(FFinstance* instance, FFstrbuf** exeNamePtr, FFstrbuf** processNamePtr, FFstrbuf** errorPtr)
{
    static FFstrbuf exeName;
    static FFstrbuf processName;
    static FFstrbuf error;
    static bool init = false;

    if(exeNamePtr != NULL)
        *exeNamePtr = &exeName;

    if(processNamePtr != NULL)
        *processNamePtr = &processName;

    if(errorPtr != NULL)
        *errorPtr = &error;

    if(init)
        return;

    ffStrbufInit(&exeName);
    ffStrbufInit(&processName);

    char ppid[256];
    sprintf(ppid, "%i", getppid());

    getTerminalName(instance, ppid, &exeName, &processName, &error);

    init = true;
}

void ffPrintTerminal(FFinstance* instance)
{
    FFstrbuf* exeName;
    FFstrbuf* processName;
    FFstrbuf* error;

    ffGetTerminal(instance, &exeName, &processName, &error);

    FFstrbuf* name;

    if(ffStrbufStartsWith(exeName, processName))
        name = exeName;
    else
        name = processName;

    if(instance->config.terminalFormat.length == 0)
    {
        if(error->length > 0)
        {
            ffPrintError(instance, "Terminal", error->chars);
            return;
        }

        ffPrintLogoAndKey(instance, "Terminal");
        ffStrbufPutTo(name, stdout);
    }
    else
    {
        ffPrintFormatString(instance, "Terminal", &instance->config.terminalFormat, 3,
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, exeName},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, processName},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, name}
        );
    }
}

#include "fastfetch.h"

#include <dirent.h>

void ffCalculateWM(FFinstance* instance, FFstrbuf** prettyNamePtr, FFstrbuf** processNamePtr, FFstrbuf** errorPtr)
{
    static FFstrbuf prettyName;
    static FFstrbuf processName;
    static FFstrbuf error;
    static bool init = false;

    if(prettyNamePtr != NULL)
        *prettyNamePtr = &prettyName;

    if(processNamePtr != NULL)
        *processNamePtr = &processName;

    if(errorPtr != NULL)
        *errorPtr = &error;

    if(init)
        return;
    init = true;

    ffStrbufInit(&prettyName);
    ffStrbufInit(&processName);
    ffStrbufInit(&error);

    DIR* proc = opendir("/proc/");
    if(proc == NULL)
    {
        ffStrbufSetS(&error, "opendir(\"/proc/\") == NULL");
        return;
    }

    struct dirent* dirent;

    while((dirent = readdir(proc)) != NULL)
    {
        if(dirent->d_type != DT_DIR)
            continue;

        char path[20];
        sprintf(path, "/proc/%.8s/comm", dirent->d_name);

        ffGetFileContent(path, &processName);

        if(ffStrbufIgnCaseCompS(&processName, "kwin_wayland") == 0 || ffStrbufIgnCaseCompS(&processName, "kwin_x11") == 0)
            ffStrbufSetS(&prettyName, "KWin");

        if(prettyName.length > 0)
            break;
    }

    closedir(proc);

    if(prettyName.length == 0)
    {
        ffStrbufSetS(&error, "No process name matches the name of known display managers");
        return;
    }
}

void ffPrintWM(FFinstance* instance)
{
    FFstrbuf* prettyName;
    FFstrbuf* processName;
    FFstrbuf* error;

    ffCalculateWM(instance, &prettyName, &processName, &error);

    if(error->length > 0)
    {
        ffPrintError(instance, &instance->config.wmKey, "WM", error->chars);
        return;
    }

    if(instance->config.wmFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, &instance->config.wmKey, "WM");
        ffStrbufPutTo(prettyName, stdout);
    }
    else
    {
        ffPrintFormatString(instance, &instance->config.wmKey, "WM", &instance->config.wmFormat, 2,
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, processName},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, prettyName}
        );
    }
}

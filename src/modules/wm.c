#include "fastfetch.h"

#include <dirent.h>

void ffPrintWM(FFinstance* instance)
{
    DIR* proc = opendir("/proc/");
    if(proc == NULL)
    {
        ffPrintError(instance, "WM", "opendir(\"/proc/\") == NULL");
        return;
    }

    struct dirent* dirent;

    FF_STRBUF_CREATE(processName);
    FF_STRBUF_CREATE(prettyName);

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

    if(instance->config.wmFormat.length == 0)
    {
        if(prettyName.length == 0)
        {
            ffPrintError(instance, "WM", "No process name matches the name of known display managers");
            return;
        }

        ffPrintLogoAndKey(instance, "WM");
        ffStrbufPutTo(&prettyName, stdout);
    }
    else
    {
        ffPrintFormatString(instance, "WM", &instance->config.wmFormat, 2,
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, &processName},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, &prettyName}
        );
    }

    ffStrbufDestroy(&processName);
    ffStrbufDestroy(&prettyName);
}

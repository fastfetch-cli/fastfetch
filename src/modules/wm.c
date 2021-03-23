#include "fastfetch.h"

#include <dirent.h>

static bool parseProcessName(FFstrbuf* name)
{
    if(ffStrbufIgnCaseCompS(name, "kwin_wayland") == 0 || ffStrbufIgnCaseCompS(name, "kwin_x11") == 0)
        ffStrbufSetS(name, "KWin");
    else
        return false;

    return true;        
}

void ffPrintWM(FFinstance* instance)
{
    DIR* proc = opendir("/proc/");
    if(proc == NULL)
    {
        ffPrintError(instance, "WM", "opendir(\"/proc/\") == NULL");
        return;
    }

    struct dirent* dirent;

    FF_STRBUF_CREATE(name);

    bool found = false;

    while((dirent = readdir(proc)) != NULL)
    {
        if(dirent->d_type != DT_DIR)
            continue;

        char path[20];
        sprintf(path, "/proc/%.8s/comm", dirent->d_name);

        ffGetFileContent(path, &name);

        if((found = parseProcessName(&name)))
            break;
    }

    closedir(proc);

    if(!found)
    {
        ffPrintError(instance, "WM", "No process name matches the name of known display managers");
        return;
    }

    if(instance->config.wmFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, "WM");
        ffStrbufPutTo(&name, stdout);
    }
    else
    {
        FF_STRBUF_CREATE(wm);

        ffParseFormatString(&wm, &instance->config.wmFormat, 1,
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, &name}
        );

        ffPrintLogoAndKey(instance, "WM");
        ffStrbufPutTo(&wm, stdout);
        ffStrbufDestroy(&wm);
    }

    ffStrbufDestroy(&name);
}
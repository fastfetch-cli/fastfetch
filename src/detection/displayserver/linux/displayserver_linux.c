#include "displayserver_linux.h"
#include "util/stringUtils.h"

#include <dirent.h>

static void parseDRM(FFDisplayServerResult* result)
{
    const char* drmDirPath = "/sys/class/drm/";

    DIR* dirp = opendir(drmDirPath);
    if(dirp == NULL)
        return;

    FF_STRBUF_AUTO_DESTROY drmDir = ffStrbufCreateA(64);
    ffStrbufAppendS(&drmDir, drmDirPath);

    uint32_t drmDirLength = drmDir.length;

    struct dirent* entry;
    while((entry = readdir(dirp)) != NULL)
    {
        if(ffStrEquals(entry->d_name, ".") || ffStrEquals(entry->d_name, ".."))
            continue;

        ffStrbufAppendS(&drmDir, entry->d_name);
        ffStrbufAppendS(&drmDir, "/modes");

        FILE* modeFile = fopen(drmDir.chars, "r");
        if(modeFile == NULL)
        {
            ffStrbufSubstrBefore(&drmDir, drmDirLength);
            continue;
        }

        uint32_t width = 0, height = 0;

        int scanned = fscanf(modeFile, "%ux%u", &width, &height);
        if(scanned == 2 && width > 0 && height > 0)
        {
            const char* plainName = entry->d_name;
            if (ffStrStartsWith(plainName, "card"))
            {
                const char* tmp = strchr(plainName + strlen("card"), '-');
                if (tmp) plainName = tmp + 1;
            }

            FF_STRBUF_AUTO_DESTROY name = ffStrbufCreateS(plainName);
            ffdsAppendDisplay(
                result,
                width, height,
                0,
                0, 0,
                0,
                &name,
                FF_DISPLAY_TYPE_UNKNOWN,
                false
            );
        }

        fclose(modeFile);
        ffStrbufSubstrBefore(&drmDir, drmDirLength);
    }

    closedir(dirp);
}

void ffConnectDisplayServerImpl(FFDisplayServerResult* ds, const FFinstance* instance)
{
    ffStrbufInit(&ds->wmProcessName);
    ffStrbufInit(&ds->wmPrettyName);
    ffStrbufInit(&ds->wmProtocolName);
    ffStrbufInit(&ds->deProcessName);
    ffStrbufInit(&ds->dePrettyName);
    ffStrbufInit(&ds->deVersion);
    ffListInitA(&ds->displays, sizeof(FFDisplayResult), 4);

    //We try wayland as our prefered display server, as it supports the most features.
    //This method can't detect the name of our WM / DE
    ffdsConnectWayland(instance, ds);

    //Try the x11 libs, from most feature rich to least.
    //We use the display list to detect if a connection is needed.
    //They respect wmProtocolName, and only detect display if it is set.

    if(ds->displays.length == 0)
        ffdsConnectXcbRandr(instance, ds);

    if(ds->displays.length == 0)
        ffdsConnectXrandr(instance, ds);

    if(ds->displays.length == 0)
        ffdsConnectXcb(instance, ds);

    if(ds->displays.length == 0)
        ffdsConnectXlib(instance, ds);

    //This display detection method is display server independent.
    //Use it if all connections failed
    if(ds->displays.length == 0)
        parseDRM(ds);

    //This fills in missing information about WM / DE by using env vars and iterating processes
    ffdsDetectWMDE(instance, ds);
}

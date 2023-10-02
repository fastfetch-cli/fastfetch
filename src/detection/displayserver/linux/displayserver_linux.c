#include "displayserver_linux.h"
#include "common/io/io.h"
#include "common/time.h"
#include "util/edidHelper.h"
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
            ffStrbufSubstrBefore(&drmDir, drmDirLength);
            ffStrbufAppendS(&drmDir, entry->d_name);
            ffStrbufAppendS(&drmDir, "/edid");

            FF_STRBUF_AUTO_DESTROY name = ffStrbufCreate();
            uint8_t edidData[128];
            if(ffReadFileData(drmDir.chars, sizeof(edidData), edidData) == sizeof(edidData))
                ffEdidGetName(edidData, &name);
            else
            {
                const char* plainName = entry->d_name;
                if (ffStrStartsWith(plainName, "card"))
                {
                    const char* tmp = strchr(plainName + strlen("card"), '-');
                    if (tmp) plainName = tmp + 1;
                }
                ffStrbufAppendS(&name, plainName);
            }

            ffdsAppendDisplay(
                result,
                width, height,
                0,
                0, 0,
                0,
                &name,
                FF_DISPLAY_TYPE_UNKNOWN,
                false,
                0
            );
        }

        fclose(modeFile);
        ffStrbufSubstrBefore(&drmDir, drmDirLength);
    }

    closedir(dirp);
}

void ffConnectDisplayServerImpl(FFDisplayServerResult* ds)
{
    uint64_t start = ffTimeGetTick();
    printf("DS start: %lums\n", ffTimeGetTick() - start);
    ffStrbufInit(&ds->wmProcessName);
    ffStrbufInit(&ds->wmPrettyName);
    ffStrbufInit(&ds->wmProtocolName);
    ffStrbufInit(&ds->deProcessName);
    ffStrbufInit(&ds->dePrettyName);
    ffStrbufInit(&ds->deVersion);
    ffListInitA(&ds->displays, sizeof(FFDisplayResult), 4);

    if (!instance.config.dsForceDrm)
    {
        printf("DS Wayland start: %lums\n", ffTimeGetTick() - start);
        //We try wayland as our prefered display server, as it supports the most features.
        //This method can't detect the name of our WM / DE
        ffdsConnectWayland(ds);

        printf("DS XcbRandr start: %lums\n", ffTimeGetTick() - start);
        //Try the x11 libs, from most feature rich to least.
        //We use the display list to detect if a connection is needed.
        //They respect wmProtocolName, and only detect display if it is set.

        if(ds->displays.length == 0)
            ffdsConnectXcbRandr(ds);

        printf("DS Xrandr start: %lums\n", ffTimeGetTick() - start);
        if(ds->displays.length == 0)
            ffdsConnectXrandr(ds);

        printf("DS Xcb start: %lums\n", ffTimeGetTick() - start);
        if(ds->displays.length == 0)
            ffdsConnectXcb(ds);

        printf("DS Xlib start: %lums\n", ffTimeGetTick() - start);
        if(ds->displays.length == 0)
            ffdsConnectXlib(ds);
    }

    printf("DS DRM start: %lums\n", ffTimeGetTick() - start);
    //This display detection method is display server independent.
    //Use it if all connections failed
    if(ds->displays.length == 0)
        parseDRM(ds);

    printf("DS WMDE start: %lums\n", ffTimeGetTick() - start);
    //This fills in missing information about WM / DE by using env vars and iterating processes
    ffdsDetectWMDE(ds);
    printf("DS end: %lums\n", ffTimeGetTick() - start);
}

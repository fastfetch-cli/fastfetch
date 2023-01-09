#include "displayserver_linux.h"
#include "common/io.h"

#include <dirent.h>

static void parseBacklight(FFDisplayServerResult* result)
{
    if(result->resolutions.length != 1)
        return;

    //https://www.kernel.org/doc/Documentation/ABI/stable/sysfs-class-backlight
    const char* backlightDirPath = "/sys/class/backlight/";

    DIR* dirp = opendir(backlightDirPath);
    if(dirp == NULL)
        return;

    FFstrbuf backlightDir;
    ffStrbufInitA(&backlightDir, 64);
    ffStrbufAppendS(&backlightDir, backlightDirPath);

    uint32_t backlightDirLength = backlightDir.length;

    FFstrbuf buffer;
    ffStrbufInit(&buffer);

    struct dirent* entry;
    while((entry = readdir(dirp)) != NULL)
    {
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        ffStrbufAppendS(&backlightDir, entry->d_name);
        ffStrbufAppendS(&backlightDir, "/actual_brightness");
        if(ffReadFileBuffer(backlightDir.chars, &buffer))
        {
            double actualBrightness = ffStrbufToDouble(&buffer);
            ffStrbufSubstrBefore(&backlightDir, backlightDirLength);
            ffStrbufAppendS(&backlightDir, entry->d_name);
            ffStrbufAppendS(&backlightDir, "/max_brightness");
            if(ffReadFileBuffer(backlightDir.chars, &buffer))
            {
                double maxBrightness = ffStrbufToDouble(&buffer);
                FF_LIST_FOR_EACH(FFResolutionResult, resolution, result->resolutions)
                {
                    resolution->brightness = (int) (actualBrightness * 100 / maxBrightness);
                }
            }

            break;
        }
    }

    closedir(dirp);
    ffStrbufDestroy(&backlightDir);
    ffStrbufDestroy(&buffer);
}

static void parseDRM(FFDisplayServerResult* result)
{
    const char* drmDirPath = "/sys/class/drm/";

    DIR* dirp = opendir(drmDirPath);
    if(dirp == NULL)
        return;

    FFstrbuf drmDir;
    ffStrbufInitA(&drmDir, 64);
    ffStrbufAppendS(&drmDir, drmDirPath);

    uint32_t drmDirLength = drmDir.length;

    struct dirent* entry;
    while((entry = readdir(dirp)) != NULL)
    {
        ffStrbufAppendS(&drmDir, entry->d_name);
        ffStrbufAppendS(&drmDir, "/modes");

        FILE* modeFile = fopen(drmDir.chars, "r");
        if(modeFile == NULL)
        {
            ffStrbufSubstrBefore(&drmDir, drmDirLength);
            continue;
        }

        FFResolutionResult* resolution = ffListAdd(&result->resolutions);
        resolution->width = 0;
        resolution->height = 0;
        resolution->refreshRate = 0;

        int scanned = fscanf(modeFile, "%ux%u", &resolution->width, &resolution->height);
        if(scanned < 2 || resolution->width == 0 || resolution->height == 0)
            --result->resolutions.length;

        fclose(modeFile);
        ffStrbufSubstrBefore(&drmDir, drmDirLength);
    }

    closedir(dirp);
    ffStrbufDestroy(&drmDir);
}

void ffConnectDisplayServerImpl(FFDisplayServerResult* ds, const FFinstance* instance)
{
    ffStrbufInit(&ds->wmProcessName);
    ffStrbufInit(&ds->wmPrettyName);
    ffStrbufInit(&ds->wmProtocolName);
    ffStrbufInit(&ds->deProcessName);
    ffStrbufInit(&ds->dePrettyName);
    ffStrbufInit(&ds->deVersion);
    ffListInitA(&ds->resolutions, sizeof(FFResolutionResult), 4);

    //We try wayland as our prefered display server, as it supports the most features.
    //This method can't detect the name of our WM / DE
    ffdsConnectWayland(instance, ds);

    //Try the x11 libs, from most feature rich to least.
    //We use the resolution list to detect if a connection is needed.
    //They respect wmProtocolName, and only detect resolution if it is set.

    if(ds->resolutions.length == 0)
        ffdsConnectXcbRandr(instance, ds);

    if(ds->resolutions.length == 0)
        ffdsConnectXrandr(instance, ds);

    if(ds->resolutions.length == 0)
        ffdsConnectXcb(instance, ds);

    if(ds->resolutions.length == 0)
        ffdsConnectXlib(instance, ds);

    //This resolution detection method is display server independent.
    //Use it if all connections failed
    if(ds->resolutions.length == 0)
        parseDRM(ds);

    //This fills in missing information about WM / DE by using env vars and iterating processes
    ffdsDetectWMDE(instance, ds);

    parseBacklight(ds);
}

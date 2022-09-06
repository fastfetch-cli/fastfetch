#include "displayserver_linux.h"

#include <dirent.h>

uint32_t ffdsParseRefreshRate(int32_t refreshRate)
{
    if(refreshRate <= 0)
        return 0;

    int remainder = refreshRate % 5;
    if(remainder >= 3)
        refreshRate += (5 - remainder);
    else
        refreshRate -= remainder;

    //All other typicall refresh rates are dividable by 5
    if(refreshRate == 145)
        refreshRate = 144;

    return (uint32_t) refreshRate;
}

bool ffdsAppendResolution(FFDisplayServerResult* result, uint32_t width, uint32_t height, uint32_t refreshRate)
{
    if(width == 0 || height == 0)
        return false;

    FFResolutionResult* resolution = ffListAdd(&result->resolutions);
    resolution->width = width;
    resolution->height = height;
    resolution->refreshRate = refreshRate;

    return true;
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

        int scanned = fscanf(modeFile, "%ix%i", &resolution->width, &resolution->height);
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
}

#include "displayServer.h"
#include <pthread.h>
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

const FFDisplayServerResult* ffConnectDisplayServer(const FFinstance* instance)
{
    static FFDisplayServerResult result;

    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    static bool init = false;
    pthread_mutex_lock(&mutex);
    if(init)
    {
        pthread_mutex_unlock(&mutex);
        return &result;
    }
    init = true;

    ffStrbufInit(&result.wmProcessName);
    ffStrbufInit(&result.wmPrettyName);
    ffStrbufInit(&result.wmProtocolName);
    ffStrbufInit(&result.deProcessName);
    ffStrbufInit(&result.dePrettyName);
    ffStrbufInit(&result.deVersion);
    ffListInitA(&result.resolutions, sizeof(FFResolutionResult), 4);

    //We try wayland as our prefered display server, as it supports the most features.
    //This method can't detect the name of our WM / DE
    ffdsConnectWayland(instance, &result);

    //Try the x11 libs, from most feature rich to least.
    //We use the resolution list to detect if a connection is needed.
    //They respect wmProtocolName, and only detect resolution if it is set.

    if(result.resolutions.length == 0)
        ffdsConnectXrandr(instance, &result);

    if(result.resolutions.length == 0)
        ffdsConnectXlib(instance, &result);

    //This resolution detection method is display server independent.
    //Use it if all connections failed
    if(result.resolutions.length == 0)
        parseDRM(&result);

    //This fills in missing information about WM / DE by using env vars and iterating processes
    ffdsDetectWMDE(instance, &result);

    pthread_mutex_unlock(&mutex);
    return &result;
}

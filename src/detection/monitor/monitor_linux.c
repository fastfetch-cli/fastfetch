#include "monitor.h"

#include "common/io/io.h"
#include "util/edidHelper.h"
#include "util/stringUtils.h"

#include <dirent.h>

const char* ffDetectMonitor(FFlist* results)
{
    const char* drmDirPath = "/sys/class/drm/";

    DIR* dirp = opendir(drmDirPath);
    if(dirp == NULL)
        return "opendir(drmDirPath) == NULL";

    FF_STRBUF_AUTO_DESTROY drmDir = ffStrbufCreateA(64);
    ffStrbufAppendS(&drmDir, drmDirPath);

    uint32_t drmDirLength = drmDir.length;

    struct dirent* entry;
    while((entry = readdir(dirp)) != NULL)
    {
        if(ffStrEquals(entry->d_name, ".") || ffStrEquals(entry->d_name, ".."))
            continue;

        ffStrbufAppendS(&drmDir, entry->d_name);
        ffStrbufAppendS(&drmDir, "/edid");

        uint8_t edidData[128];
        if(ffReadFileData(drmDir.chars, sizeof(edidData), edidData) != sizeof(edidData))
        {
            ffStrbufSubstrBefore(&drmDir, drmDirLength);
            continue;
        }

        uint32_t width, height;
        ffEdidGetPhysicalResolution(edidData, &width, &height);
        if (width != 0 && height != 0)
        {
            FFMonitorResult* display = (FFMonitorResult*) ffListAdd(results);
            display->width = width;
            display->height = height;
            ffStrbufInit(&display->name);
            ffEdidGetName(edidData, &display->name);
            ffEdidGetPhysicalSize(edidData, &display->physicalWidth, &display->physicalHeight);
        }

        ffStrbufSubstrBefore(&drmDir, drmDirLength);
    }

    closedir(dirp);
    return NULL;
}

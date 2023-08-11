#include "monitor.h"

#include "common/io/io.h"
#include "util/edidHelper.h"
#include "util/stringUtils.h"
#include "util/mallocHelper.h"

#include <dirent.h>
#include <sys/stat.h>

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

        struct stat fileStat;
        if (stat(drmDir.chars, &fileStat) < 0 || fileStat.st_size == 0 || fileStat.st_size % 128 != 0)
        {
            ffStrbufSubstrBefore(&drmDir, drmDirLength);
            continue;
        }

        FF_AUTO_FREE uint8_t* edidData = malloc((size_t) fileStat.st_size);
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
            display->hdrCompatible = ffEdidGetHdrCompatible(edidData, (uint32_t) fileStat.st_size);
        }

        ffStrbufSubstrBefore(&drmDir, drmDirLength);
    }

    closedir(dirp);
    return NULL;
}

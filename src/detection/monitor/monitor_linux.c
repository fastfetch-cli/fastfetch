#include "monitor.h"

#include "common/io/io.h"
#include "util/edidHelper.h"
#include "util/stringUtils.h"

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
        uint32_t drmDirWithDnameLength = drmDir.length;

        ffStrbufAppendS(&drmDir, "/status");
        char status = 'd'; // disconnected
        ffReadFileData(drmDir.chars, sizeof(status), &status);
        if (status != 'c') // connected
        {
            ffStrbufSubstrBefore(&drmDir, drmDirLength);
            continue;
        }

        ffStrbufSubstrBefore(&drmDir, drmDirWithDnameLength);
        ffStrbufAppendS(&drmDir, "/edid");

        uint8_t edidData[512];
        ssize_t edidLength = ffReadFileData(drmDir.chars, sizeof(edidData), edidData);
        if(edidLength <= 0 || edidLength % 128 != 0)
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
            display->hdrCompatible = ffEdidGetHdrCompatible(edidData, (uint32_t) edidLength);
        }

        ffStrbufSubstrBefore(&drmDir, drmDirLength);
    }

    closedir(dirp);
    return NULL;
}

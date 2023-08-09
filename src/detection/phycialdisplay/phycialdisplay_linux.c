#include "phycialdisplay.h"

#include "common/io/io.h"
#include "util/edidHelper.h"
#include "util/stringUtils.h"

#include <dirent.h>

const char* ffDetectPhycialDisplay(FFlist* results)
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
        ffEdidGetPhycialResolution(edidData, &width, &height);
        if (width != 0 && height != 0)
        {
            const char* plainName = entry->d_name;
            if (ffStrStartsWith(plainName, "card"))
            {
                const char* tmp = strchr(plainName + strlen("card"), '-');
                if (tmp) plainName = tmp + 1;
            }

            FFPhycialDisplayResult* display = (FFPhycialDisplayResult*) ffListAdd(results);
            display->width = width;
            display->height = height;
            ffStrbufInitS(&display->name, plainName);
            ffEdidGetPhycialSize(edidData, &display->phycialWidth, &display->phycialHeight);
        }

        ffStrbufSubstrBefore(&drmDir, drmDirLength);
    }

    closedir(dirp);
    return NULL;
}

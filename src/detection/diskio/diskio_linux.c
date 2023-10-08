#include "diskio.h"
#include "common/io/io.h"
#include "util/stringUtils.h"

#include <ctype.h>
#include <limits.h>

const char* ffDiskIOGetIoCounters(FFlist* result, FFDiskIOOptions* options)
{
    FF_STRBUF_AUTO_DESTROY baseDir = ffStrbufCreateS("/dev/disk/by-id/");
    uint32_t baseDirLength = baseDir.length;

    FF_AUTO_CLOSE_DIR DIR* dirp = opendir(baseDir.chars);
    if(dirp == NULL)
        return "opendir(batteryDir) == NULL";

    struct dirent* entry;
    while ((entry = readdir(dirp)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;

        char* part = strstr(entry->d_name, "-part");
        if (part && isdigit(part[strlen("-part")]))
            continue;

        if (ffStrStartsWith(entry->d_name, "nvme-eui.")) // NVMe drive indentifier
            continue;

        // Other exceptions?

        ffStrbufAppendS(&baseDir, entry->d_name);
        char pathDev[PATH_MAX];
        ssize_t pathLen = readlink(baseDir.chars, pathDev, PATH_MAX);
        ffStrbufSubstrBefore(&baseDir, baseDirLength);

        if (pathLen < 0) continue;
        pathDev[pathLen] = '\0'; // ../../nvme0n1

        char pathDev1[PATH_MAX];
        const char* devName = basename(pathDev);
        snprintf(pathDev1, PATH_MAX, "/dev/%s", devName);

        // Avoid duplicated disks
        bool flag = false;
        FF_LIST_FOR_EACH(FFDiskIOResult, disk, *result)
        {
            if (ffStrbufEqualS(&disk->devPath, pathDev1))
            {
                flag = true;
                break;
            }
        }
        if (flag) continue;

        char pathSysBlockStat[PATH_MAX];
        snprintf(pathSysBlockStat, PATH_MAX, "/sys/block/%s/stat", devName);

        FF_AUTO_CLOSE_FILE FILE* sysBlockStat = fopen(pathSysBlockStat, "r");
        if (!sysBlockStat) continue;

        // I/Os merges sectors ticks ...
        uint64_t nRead, sectorRead, nWritten, sectorWritten;
        if (fscanf(sysBlockStat, "%lu%*u%lu%*u%lu%*u%lu%*u", &nRead, &sectorRead, &nWritten, &sectorWritten) == 0)
            continue;

        FFDiskIOResult* device = (FFDiskIOResult*) ffListAdd(result);
        char* slash = strchr(entry->d_name, '-');
        if (slash)
        {
            char* slash2 = strchr(slash + 1, '-');
            if (slash2)
                ffStrbufInitNS(&device->name, (uint32_t) (slash2 - slash - 1), slash + 1);
            else
                ffStrbufInitS(&device->name, slash + 1);
            ffStrbufInitNS(&device->type, (uint32_t) (slash - entry->d_name), entry->d_name);
        }
        else
        {
            ffStrbufInitS(&device->name, entry->d_name);
            ffStrbufInit(&device->type);
        }
        ffStrbufReplaceAllC(&device->name, '_', ' ');

        if (options->namePrefix.length && !ffStrbufStartsWith(&device->name, &options->namePrefix))
        {
            ffStrbufDestroy(&device->name);
            ffStrbufDestroy(&device->type);
            result->length--;
            continue;
        }

        ffStrbufInitS(&device->devPath, pathDev1);
        device->bytesRead = sectorRead * 512;
        device->bytesWritten = sectorWritten * 512;
        device->readCount = nRead;
        device->writeCount = nWritten;
    }

    return NULL;
}

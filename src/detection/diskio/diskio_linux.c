#include "diskio.h"
#include "common/io/io.h"
#include "util/stringUtils.h"

#include <ctype.h>
#include <limits.h>

const char* ffDiskIOGetIoCounters(FFlist* result, FFDiskIOOptions* options)
{
    FF_AUTO_CLOSE_DIR DIR* sysBlockDirp = opendir("/sys/block/");
    if(sysBlockDirp == NULL)
        return "opendir(\"/sys/block/\") == NULL";

    struct dirent* sysBlockEntry;
    while ((sysBlockEntry = readdir(sysBlockDirp)) != NULL)
    {
        const char* const devName = sysBlockEntry->d_name;

        if (devName[0] == '.')
            continue;

        if (ffStrStartsWith(devName, "dm-")) // LVM logical partitions
            continue;

        char pathSysBlock[PATH_MAX];

        char blockId[256] = "";
        {
            FF_AUTO_CLOSE_DIR DIR* devDiskDirp = opendir("/dev/disk/by-id");
            if (devDiskDirp)
            {
                struct dirent* devDiskEntry;
                while ((devDiskEntry = readdir(devDiskDirp)) != NULL)
                {
                    if (devDiskEntry->d_name[0] == '.')
                        continue;

                    char pathDevDisk[PATH_MAX];
                    snprintf(pathDevDisk, PATH_MAX, "/dev/disk/by-id/%s", devDiskEntry->d_name);

                    char pathDev[PATH_MAX];
                    ssize_t pathLen = readlink(pathDevDisk, pathDev, sizeof(pathDev) - 1);
                    if (pathLen > 0)
                    {
                        pathDev[pathLen] = '\0';
                        if (!ffStrEquals(basename(pathDev), devName))
                            continue;

                        if (ffStrStartsWith(devDiskEntry->d_name, "nvme-eui.")) // NVMe drive identifier
                            continue;

                        strcpy(blockId, devDiskEntry->d_name);
                        break;
                    }
                }
            }
        }

        FFDiskIOResult* device = (FFDiskIOResult*) ffListAdd(result);

        if (blockId[0])
        {
            char* slash = strchr(blockId, '-');
            if (slash)
            {
                char* slash2 = strchr(slash + 1, '-');
                if (slash2)
                    ffStrbufInitNS(&device->name, (uint32_t) (slash2 - slash - 1), slash + 1);
                else
                    ffStrbufInitS(&device->name, slash + 1);
                ffStrbufInitNS(&device->interconnect, (uint32_t) (slash - blockId), blockId);
            }
            else
            {
                ffStrbufInitS(&device->name, blockId);
                ffStrbufInit(&device->interconnect);
            }
            ffStrbufReplaceAllC(&device->name, '_', ' ');
        }
        else
        {
            ffStrbufInitS(&device->name, devName);
            ffStrbufInit(&device->interconnect);
        }

        if (options->namePrefix.length && !ffStrbufStartsWith(&device->name, &options->namePrefix))
        {
            ffStrbufDestroy(&device->name);
            ffStrbufDestroy(&device->interconnect);
            result->length--;
            continue;
        }

        // I/Os merges sectors ticks ...
        uint64_t nRead, sectorRead, nWritten, sectorWritten;
        {
            char sysBlockStat[PROC_FILE_BUFFSIZ];
            snprintf(pathSysBlock, PATH_MAX, "/sys/block/%s/stat", devName);
            ssize_t fileSize = ffReadFileData(pathSysBlock, sizeof(sysBlockStat) - 1, sysBlockStat);
            if (fileSize <= 0) continue;
            sysBlockStat[fileSize] = '\0';
            if (sscanf(sysBlockStat, "%lu%*u%lu%*u%lu%*u%lu%*u", &nRead, &sectorRead, &nWritten, &sectorWritten) <= 0)
                continue;
        }

        ffStrbufInitF(&device->devPath, "/dev/%s", devName);
        device->bytesRead = sectorRead * 512;
        device->bytesWritten = sectorWritten * 512;
        device->readCount = nRead;
        device->writeCount = nWritten;

        {
            snprintf(pathSysBlock, PATH_MAX, "/sys/block/%s/queue/rotational", devName);
            char isRotationalChar = '1';
            if (ffReadFileData(pathSysBlock, 1, &isRotationalChar))
                device->type = isRotationalChar = '1' ? FF_DISKIO_PHYSICAL_TYPE_HDD : FF_DISKIO_PHYSICAL_TYPE_SSD;
            else
                device->type = FF_DISKIO_PHYSICAL_TYPE_UNKNOWN;
        }

        {
            snprintf(pathSysBlock, PATH_MAX, "/sys/block/%s/size", devName);
            ssize_t fileSize = ffReadFileData(pathSysBlock, sizeof(blockId) - 1, blockId);
            if (fileSize > 0)
            {
                blockId[fileSize] = 0;
                device->size = (uint64_t) strtoul(blockId, NULL, 10) * 512;
            }
            else
                device->size = 0;
        }
    }

    return NULL;
}

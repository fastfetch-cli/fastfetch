#include "diskio.h"
#include "common/io/io.h"
#include "common/properties.h"
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


        char pathSysBlock[PATH_MAX];
        snprintf(pathSysBlock, PATH_MAX, "/sys/block/%s", devName);

        char pathSysDeviceReal[PATH_MAX] = "";
        readlink(pathSysBlock, pathSysDeviceReal, sizeof(pathSysDeviceReal) - 1);

        if (strstr(pathSysDeviceReal, "/virtual/")) // virtual device
            continue;

        snprintf(pathSysBlock, PATH_MAX, "/sys/block/%s/device", devName);
        if (!ffPathExists(pathSysBlock, FF_PATHTYPE_DIRECTORY))
            continue;

        FFDiskIOResult* device = (FFDiskIOResult*) ffListAdd(result);
        ffStrbufInit(&device->name);

        {
            snprintf(pathSysBlock, PATH_MAX, "/sys/block/%s/device/vendor", devName);
            ffAppendFileBuffer(pathSysBlock, &device->name);
            if (device->name.length > 0)
                ffStrbufAppendC(&device->name, ' ');

            snprintf(pathSysBlock, PATH_MAX, "/sys/block/%s/device/model", devName);
            ffAppendFileBuffer(pathSysBlock, &device->name);
            ffStrbufTrim(&device->name, ' ');

            if (device->name.length == 0)
                ffStrbufSetS(&device->name, devName);

            if (options->namePrefix.length && !ffStrbufStartsWith(&device->name, &options->namePrefix))
            {
                ffStrbufDestroy(&device->name);
                result->length--;
                continue;
            }
        }

        {
            ffStrbufInit(&device->interconnect);
            if (strstr(pathSysDeviceReal, "/usb") != NULL)
                ffStrbufSetS(&device->interconnect, "usb");
            else
            {
                snprintf(pathSysBlock, PATH_MAX, "/sys/block/%s/device/transport", devName);
                if (!ffAppendFileBuffer(pathSysBlock, &device->interconnect))
                {
                    snprintf(pathSysBlock, PATH_MAX, "/sys/block/%s/device/uevent", devName);
                    if (ffParsePropFile(pathSysBlock, "DEVTYPE=", &device->interconnect))
                        ffStrbufSubstrBeforeLastC(&device->interconnect, '_');
                }
            }
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
                device->type = isRotationalChar == '1' ? FF_DISKIO_PHYSICAL_TYPE_HDD : FF_DISKIO_PHYSICAL_TYPE_SSD;
            else
                device->type = FF_DISKIO_PHYSICAL_TYPE_UNKNOWN;
        }

        {
            snprintf(pathSysBlock, PATH_MAX, "/sys/block/%s/size", devName);
            char blkSize[32];
            ssize_t fileSize = ffReadFileData(pathSysBlock, sizeof(blkSize) - 1, blkSize);
            if (fileSize > 0)
            {
                blkSize[fileSize] = 0;
                device->size = (uint64_t) strtoul(blkSize, NULL, 10) * 512;
            }
            else
                device->size = 0;
        }

        {
            ffStrbufInit(&device->serial);
            snprintf(pathSysBlock, PATH_MAX, "/sys/block/%s/serial", devName);
            ffReadFileBuffer(pathSysBlock, &device->serial);
        }
    }

    return NULL;
}

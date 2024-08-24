#include "diskio.h"
#include "common/io/io.h"
#include "common/properties.h"
#include "util/stringUtils.h"

#include <ctype.h>
#include <limits.h>
#include <inttypes.h>

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

        char pathSysDeviceReal[PATH_MAX];
        ssize_t pathLength = readlink(pathSysBlock, pathSysDeviceReal, sizeof(pathSysDeviceReal) - 1);
        if (pathLength < 0)
            continue;
        pathSysDeviceReal[pathLength] = '\0';

        if (strstr(pathSysDeviceReal, "/virtual/")) // virtual device
            continue;

        snprintf(pathSysBlock, PATH_MAX, "/sys/block/%s/device", devName);
        if (!ffPathExists(pathSysBlock, FF_PATHTYPE_DIRECTORY))
            continue;

        FF_STRBUF_AUTO_DESTROY name = ffStrbufCreate();

        {
            snprintf(pathSysBlock, PATH_MAX, "/sys/block/%s/device/vendor", devName);
            if (ffAppendFileBuffer(pathSysBlock, &name))
            {
                ffStrbufTrimRightSpace(&name);
                if (name.length > 0)
                    ffStrbufAppendC(&name, ' ');
            }

            snprintf(pathSysBlock, PATH_MAX, "/sys/block/%s/device/model", devName);
            ffAppendFileBuffer(pathSysBlock, &name);
            ffStrbufTrimRightSpace(&name);

            if (name.length == 0)
                ffStrbufSetS(&name, devName);
            else if (ffStrStartsWith(devName, "nvme"))
            {
                int devid, nsid;
                if (sscanf(devName, "nvme%dn%d", &devid, &nsid) == 2)
                {
                    bool multiNs = nsid > 1;
                    if (!multiNs)
                    {
                        snprintf(pathSysBlock, PATH_MAX, "/sys/block/%s/device/nvme%dn2", devName, devid);
                        multiNs = ffPathExists(pathSysBlock, FF_PATHTYPE_DIRECTORY);
                    }
                    if (multiNs)
                    {
                        // In Asahi Linux, there are multiple namespaces for the same NVMe drive.
                        ffStrbufAppendF(&name, " - %d", nsid);
                    }
                }
            }

            if (options->namePrefix.length && !ffStrbufStartsWith(&name, &options->namePrefix))
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
            if (sscanf(sysBlockStat, "%" PRIu64 "%*u%" PRIu64 "%*u%" PRIu64 "%*u%" PRIu64 "%*u", &nRead, &sectorRead, &nWritten, &sectorWritten) <= 0)
                continue;
        }

        FFDiskIOResult* device = (FFDiskIOResult*) ffListAdd(result);
        ffStrbufInitMove(&device->name, &name);
        ffStrbufInitF(&device->devPath, "/dev/%s", devName);
        device->bytesRead = sectorRead * 512;
        device->bytesWritten = sectorWritten * 512;
        device->readCount = nRead;
        device->writeCount = nWritten;
    }

    return NULL;
}

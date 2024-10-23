#include "diskio.h"
#include "common/io/io.h"
#include "common/properties.h"
#include "util/stringUtils.h"

#include <ctype.h>
#include <limits.h>
#include <inttypes.h>
#include <fcntl.h>

static void parseDiskIOCounters(int dfd, const char* devName, FFlist* result, FFDiskIOOptions* options)
{
    FF_AUTO_CLOSE_FD int devfd = openat(dfd, "device", O_RDONLY | O_CLOEXEC | O_PATH | O_DIRECTORY);
    if (devfd < 0) return; // virtual device

    FF_STRBUF_AUTO_DESTROY name = ffStrbufCreate();

    {
        if (ffAppendFileBufferRelative(devfd, "vendor", &name))
        {
            ffStrbufTrimRightSpace(&name);
            if (name.length > 0)
                ffStrbufAppendC(&name, ' ');
        }

        if (ffAppendFileBufferRelative(devfd, "model", &name))
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
                    char pathSysBlock[16];
                    snprintf(pathSysBlock, ARRAY_SIZE(pathSysBlock), "nvme%dn2", devid);
                    multiNs = faccessat(devfd, pathSysBlock, F_OK, 0) == 0;
                }
                if (multiNs)
                {
                    // In Asahi Linux, there are multiple namespaces for the same NVMe drive.
                    ffStrbufAppendF(&name, " - %d", nsid);
                }
            }
        }

        if (options->namePrefix.length && !ffStrbufStartsWith(&name, &options->namePrefix))
            return;
    }

    // I/Os merges sectors ticks ...
    uint64_t nRead, sectorRead, nWritten, sectorWritten;
    {
        char sysBlockStat[PROC_FILE_BUFFSIZ];
        ssize_t fileSize = ffReadFileDataRelative(dfd, "stat", ARRAY_SIZE(sysBlockStat) - 1, sysBlockStat);
        if (fileSize <= 0) return;
        sysBlockStat[fileSize] = '\0';
        if (sscanf(sysBlockStat, "%" PRIu64 "%*u%" PRIu64 "%*u%" PRIu64 "%*u%" PRIu64 "%*u", &nRead, &sectorRead, &nWritten, &sectorWritten) <= 0)
            return;
    }

    FFDiskIOResult* device = (FFDiskIOResult*) ffListAdd(result);
    ffStrbufInitMove(&device->name, &name);
    ffStrbufInitF(&device->devPath, "/dev/%s", devName);
    device->bytesRead = sectorRead * 512;
    device->bytesWritten = sectorWritten * 512;
    device->readCount = nRead;
    device->writeCount = nWritten;
}

const char* ffDiskIOGetIoCounters(FFlist* result, FFDiskIOOptions* options)
{
    FF_AUTO_CLOSE_DIR DIR* sysBlockDirp = opendir("/sys/block/");
    if(sysBlockDirp == NULL)
        return "opendir(\"/sys/block/\") == NULL";

    struct dirent* sysBlockEntry;
    while ((sysBlockEntry = readdir(sysBlockDirp)) != NULL)
    {
        const char* const devName = sysBlockEntry->d_name;

        if (devName[0] == '.') continue;;

        FF_AUTO_CLOSE_FD int dfd = openat(dirfd(sysBlockDirp), devName, O_RDONLY | O_CLOEXEC | O_PATH | O_DIRECTORY);
        if (dfd > 0) parseDiskIOCounters(dfd, devName, result, options);
    }

    return NULL;
}

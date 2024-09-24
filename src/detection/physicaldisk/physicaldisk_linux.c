#include "physicaldisk.h"
#include "common/io/io.h"
#include "common/properties.h"
#include "util/stringUtils.h"

#include <ctype.h>
#include <limits.h>

static double detectNvmeTemp(const char* devName)
{
    char pathSysBlock[PATH_MAX];
    int index = snprintf(pathSysBlock, PATH_MAX, "/sys/block/%s/device/hwmon$/temp1_input", devName);
    if (index <= 0) return FF_PHYSICALDISK_TEMP_UNSET;
    index -= (int) sizeof("/temp1_input");

    for (char c = '0'; c <= '9'; c++) // hopefully there's only one digit
    {
        pathSysBlock[index] = c;
        char buffer[64];
        ssize_t size = ffReadFileData(pathSysBlock, sizeof(buffer), buffer);
        if (size > 0)
        {
            buffer[size] = '\0';
            double temp = strtod(buffer, NULL);
            return temp > 0 ? temp / 1000 : FF_PHYSICALDISK_TEMP_UNSET;
        }
    }

    return FF_PHYSICALDISK_TEMP_UNSET;
}

const char* ffDetectPhysicalDisk(FFlist* result, FFPhysicalDiskOptions* options)
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

        FFPhysicalDiskResult* device = (FFPhysicalDiskResult*) ffListAdd(result);
        device->type = FF_PHYSICALDISK_TYPE_NONE;
        ffStrbufInit(&device->name);

        {
            snprintf(pathSysBlock, PATH_MAX, "/sys/block/%s/device/vendor", devName);
            if (ffAppendFileBuffer(pathSysBlock, &device->name))
            {
                ffStrbufTrimRightSpace(&device->name);
                if (device->name.length > 0)
                    ffStrbufAppendC(&device->name, ' ');
            }

            snprintf(pathSysBlock, PATH_MAX, "/sys/block/%s/device/model", devName);
            ffAppendFileBuffer(pathSysBlock, &device->name);
            ffStrbufTrimRightSpace(&device->name);

            if (device->name.length == 0)
                ffStrbufSetS(&device->name, devName);
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
                        ffStrbufAppendF(&device->name, " - %d", nsid);
                    }
                }
            }

            if (options->namePrefix.length && !ffStrbufStartsWith(&device->name, &options->namePrefix))
            {
                ffStrbufDestroy(&device->name);
                result->length--;
                continue;
            }
        }

        ffStrbufInitF(&device->devPath, "/dev/%s", devName);

        {
            ffStrbufInit(&device->interconnect);
            if (strstr(pathSysDeviceReal, "/usb") != NULL)
                ffStrbufSetS(&device->interconnect, "USB");
            else if (strstr(pathSysDeviceReal, "/nvme") != NULL)
                ffStrbufSetS(&device->interconnect, "NVMe");
            else if (strstr(pathSysDeviceReal, "/ata") != NULL)
                ffStrbufSetS(&device->interconnect, "ATA");
            else if (strstr(pathSysDeviceReal, "/scsi") != NULL)
                ffStrbufSetS(&device->interconnect, "SCSI");
            else
            {
                snprintf(pathSysBlock, PATH_MAX, "/sys/block/%s/device/transport", devName);
                if (ffAppendFileBuffer(pathSysBlock, &device->interconnect))
                    ffStrbufTrimRightSpace(&device->interconnect);
            }
        }

        {
            snprintf(pathSysBlock, PATH_MAX, "/sys/block/%s/queue/rotational", devName);
            char isRotationalChar = '1';
            if (ffReadFileData(pathSysBlock, 1, &isRotationalChar) > 0)
                device->type |= isRotationalChar == '1' ? FF_PHYSICALDISK_TYPE_HDD : FF_PHYSICALDISK_TYPE_SSD;
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
            char removableChar = '0';
            snprintf(pathSysBlock, PATH_MAX, "/sys/block/%s/removable", devName);
            if (ffReadFileData(pathSysBlock, 1, &removableChar) > 0)
                device->type |= removableChar == '1' ? FF_PHYSICALDISK_TYPE_REMOVABLE : FF_PHYSICALDISK_TYPE_FIXED;
        }

        {
            char roChar = '0';
            snprintf(pathSysBlock, PATH_MAX, "/sys/block/%s/ro", devName);
            if (ffReadFileData(pathSysBlock, 1, &roChar) > 0)
                device->type |= roChar == '1' ? FF_PHYSICALDISK_TYPE_READONLY : FF_PHYSICALDISK_TYPE_READWRITE;
        }

        {
            ffStrbufInit(&device->serial);
            snprintf(pathSysBlock, PATH_MAX, "/sys/block/%s/device/serial", devName);
            if (ffReadFileBuffer(pathSysBlock, &device->serial))
                ffStrbufTrimRightSpace(&device->serial);
        }

        {
            ffStrbufInit(&device->revision);
            snprintf(pathSysBlock, PATH_MAX, "/sys/block/%s/device/firmware_rev", devName);
            if (ffReadFileBuffer(pathSysBlock, &device->revision))
                ffStrbufTrimRightSpace(&device->revision);
            else
            {
                snprintf(pathSysBlock, PATH_MAX, "/sys/block/%s/device/rev", devName);
                if (ffReadFileBuffer(pathSysBlock, &device->revision))
                    ffStrbufTrimRightSpace(&device->revision);
            }
        }

        if (options->temp)
            device->temperature = detectNvmeTemp(devName);
        else
            device->temperature = FF_PHYSICALDISK_TEMP_UNSET;
    }

    return NULL;
}

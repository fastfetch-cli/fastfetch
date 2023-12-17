#include "diskio.h"
#include "common/io/io.h"
#include "util/windows/registry.h"
#include "util/windows/unicode.h"

#include <winioctl.h>

const char* ffDiskIOGetIoCounters(FFlist* result, FFDiskIOOptions* options)
{
    FF_HKEY_AUTO_DESTROY hKey = NULL;
    if (!ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\disk\\Enum", &hKey, NULL))
        return "ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, L\"SYSTEM\\CurrentControlSet\\Services\\disk\\Enum\") failed";

    uint32_t nDevices;
    if (!ffRegReadUint(hKey, L"Count", &nDevices, NULL))
        return "ffRegReadUint(hKey, L\"Count\", &nDevices) failed";

    wchar_t szDevice[32] = L"\\\\.\\PhysicalDrive";
    wchar_t* pNum = szDevice + strlen("\\\\.\\PhysicalDrive");
    for (uint32_t idev = 0; idev <= nDevices; ++idev) {
        _ultow(idev, pNum, 10);

        FF_AUTO_CLOSE_FD HANDLE hDevice = CreateFileW(szDevice, FILE_READ_ATTRIBUTES, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        if (hDevice == INVALID_HANDLE_VALUE)
            continue;

        DISK_PERFORMANCE diskPerformance;
        DWORD ioctrlSize = sizeof(diskPerformance);
        DWORD dwSize;
        if (DeviceIoControl(hDevice, IOCTL_DISK_PERFORMANCE, NULL, 0, &diskPerformance, ioctrlSize, &dwSize, NULL))
        {
            FFDiskIOResult* device = (FFDiskIOResult*) ffListAdd(result);
            ffStrbufInit(&device->name);
            ffStrbufInit(&device->interconnect);
            if (ffRegReadStrbuf(hKey, pNum, &device->name, NULL))
            {
                // SCSI\Disk&Ven_NVMe&Prod_WDC_PC_SN810_SDC\5&19cebb7&0&000000
                uint32_t index = ffStrbufFirstIndexC(&device->name, '\\');
                if (index != device->name.length)
                {
                    ffStrbufAppendNS(&device->interconnect, index, device->name.chars); // SCSI
                    ffStrbufSubstrAfter(&device->name, index);
                }
                ffStrbufSubstrBeforeLastC(&device->name, '\\');
                ffStrbufSubstrAfterFirstS(&device->name, "&Ven_");
                ffStrbufRemoveS(&device->name, "&Prod");
                ffStrbufReplaceAllC(&device->name, '_', ' ');
                ffStrbufTrim(&device->name, ' ');
            }
            else
                ffStrbufSetWS(&device->name, szDevice);

            if (options->namePrefix.length && !ffStrbufStartsWith(&device->name, &options->namePrefix))
            {
                ffStrbufDestroy(&device->name);
                ffStrbufDestroy(&device->interconnect);
                result->length--;
                continue;
            }

            ffStrbufInitWS(&device->devPath, szDevice);
            device->bytesRead = (uint64_t) diskPerformance.BytesRead.QuadPart;
            device->readCount = (uint64_t) diskPerformance.ReadCount;
            device->bytesWritten = (uint64_t) diskPerformance.BytesWritten.QuadPart;
            device->writeCount = (uint64_t) diskPerformance.WriteCount;

            DWORD retSize = 0;

            ffStrbufInit(&device->serial);
            char sddBuffer[4096];
            if(DeviceIoControl(
                hDevice,
                IOCTL_STORAGE_QUERY_PROPERTY,
                &(STORAGE_PROPERTY_QUERY) {
                    .PropertyId = StorageDeviceProperty,
                    .QueryType = PropertyStandardQuery,
                },
                sizeof(STORAGE_PROPERTY_QUERY),
                &sddBuffer,
                sizeof(sddBuffer),
                &retSize,
                NULL
            ) && retSize > 0)
            {
                STORAGE_DEVICE_DESCRIPTOR* sdd = (STORAGE_DEVICE_DESCRIPTOR*) sddBuffer;
                if (sdd->SerialNumberOffset != 0)
                {
                    ffStrbufSetS(&device->serial, (const char*) sddBuffer + sdd->SerialNumberOffset);
                    ffStrbufTrim(&device->serial, ' ');
                }
            }

            DEVICE_SEEK_PENALTY_DESCRIPTOR dspd = {};
            if(DeviceIoControl(
                hDevice,
                IOCTL_STORAGE_QUERY_PROPERTY,
                &(STORAGE_PROPERTY_QUERY) {
                    .PropertyId = StorageDeviceSeekPenaltyProperty,
                    .QueryType = PropertyStandardQuery,
                },
                sizeof(STORAGE_PROPERTY_QUERY),
                &dspd,
                sizeof(dspd),
                &retSize,
                NULL
            ) && retSize == sizeof(dspd))
                device->type = dspd.IncursSeekPenalty ? FF_DISKIO_PHYSICAL_TYPE_HDD : FF_DISKIO_PHYSICAL_TYPE_SSD;
            else
                device->type = FF_DISKIO_PHYSICAL_TYPE_UNKNOWN;

            DISK_GEOMETRY_EX dge = {};
            if(DeviceIoControl(
                hDevice,
                IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,
                NULL,
                0,
                &dge,
                sizeof(dge),
                &retSize,
                NULL))
                device->size = (uint64_t) dge.DiskSize.QuadPart;
            else
                device->size = 0;
        }
    }

    return NULL;
}

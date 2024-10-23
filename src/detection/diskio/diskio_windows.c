#include "diskio.h"
#include "common/io/io.h"
#include "util/windows/unicode.h"

#include <windows.h>
#include <winioctl.h>

static bool detectPhysicalDisk(const wchar_t* szDevice, FFlist* result, FFDiskIOOptions* options)
{
    FF_AUTO_CLOSE_FD HANDLE hDevice = CreateFileW(szDevice, FILE_READ_ATTRIBUTES, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hDevice == INVALID_HANDLE_VALUE)
        return false;

    DWORD retSize;
    char sddBuffer[4096];
    if(!DeviceIoControl(
        hDevice,
        IOCTL_STORAGE_QUERY_PROPERTY,
        &(STORAGE_PROPERTY_QUERY) {
            .PropertyId = StorageDeviceProperty,
            .QueryType = PropertyStandardQuery,
        },
        sizeof(STORAGE_PROPERTY_QUERY),
        &sddBuffer,
        ARRAY_SIZE(sddBuffer),
        &retSize,
        NULL
    ) || retSize == 0)
        return true;

    FFDiskIOResult* device = (FFDiskIOResult*) ffListAdd(result);
    STORAGE_DEVICE_DESCRIPTOR* sdd = (STORAGE_DEVICE_DESCRIPTOR*) sddBuffer;

    ffStrbufInit(&device->name);
    if (sdd->VendorIdOffset != 0)
    {
        ffStrbufSetS(&device->name, (const char*) sddBuffer + sdd->VendorIdOffset);
        ffStrbufTrim(&device->name, ' ');
    }
    if (sdd->ProductIdOffset != 0)
    {
        if (device->name.length)
            ffStrbufAppendC(&device->name, ' ');

        ffStrbufAppendS(&device->name, (const char*) sddBuffer + sdd->ProductIdOffset);
        ffStrbufTrimRight(&device->name, ' ');
    }

    if (!device->name.length)
        ffStrbufSetWS(&device->name, szDevice);

    if (options->namePrefix.length && !ffStrbufStartsWith(&device->name, &options->namePrefix))
    {
        ffStrbufDestroy(&device->name);
        result->length--;
        return true;
    }

    ffStrbufInitWS(&device->devPath, szDevice);

    DISK_PERFORMANCE dp = {};
    if (DeviceIoControl(hDevice, IOCTL_DISK_PERFORMANCE, NULL, 0, &dp, sizeof(dp), &retSize, NULL))
    {
        device->bytesRead = (uint64_t) dp.BytesRead.QuadPart;
        device->readCount = (uint64_t) dp.ReadCount;
        device->bytesWritten = (uint64_t) dp.BytesWritten.QuadPart;
        device->writeCount = (uint64_t) dp.WriteCount;
    }
    else
    {
        ffStrbufDestroy(&device->name);
        result->length--;
    }

    return true;
}

const char* ffDiskIOGetIoCounters(FFlist* result, FFDiskIOOptions* options)
{
    {
        wchar_t szPhysicalDrive[32] = L"\\\\.\\PhysicalDrive";
        wchar_t* pNum = szPhysicalDrive + strlen("\\\\.\\PhysicalDrive");
        for (uint32_t idev = 0; ; ++idev)
        {
            _ultow(idev, pNum, 10);

            if (!detectPhysicalDisk(szPhysicalDrive, result, options))
                break;
        }
    }

    {
        wchar_t szCdrom[32] = L"\\\\.\\CDROM";
        wchar_t* pNum = szCdrom + strlen("\\\\.\\CDROM");
        for (uint32_t idev = 0; ; ++idev)
        {
            _ultow(idev, pNum, 10);

            if (!detectPhysicalDisk(szCdrom, result, options))
                break;
        }
    }

    return NULL;
}

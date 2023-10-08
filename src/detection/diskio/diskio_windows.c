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
            ffStrbufInit(&device->type);
            if (ffRegReadStrbuf(hKey, pNum, &device->name, NULL))
            {
                // SCSI\Disk&Ven_NVMe&Prod_WDC_PC_SN810_SDC\5&19cebb7&0&000000
                uint32_t index = ffStrbufFirstIndexC(&device->name, '\\');
                if (index != device->name.length)
                    ffStrbufAppendNS(&device->type, index, device->name.chars); // SCSI
                ffStrbufSubstrAfterFirstS(&device->name, "&Prod_");
                ffStrbufSubstrBeforeLastC(&device->name, '\\');
                ffStrbufReplaceAllC(&device->name, '_', ' ');
            }
            else
                ffStrbufSetWS(&device->name, szDevice);

            if (options->namePrefix.length && !ffStrbufStartsWith(&device->name, &options->namePrefix))
            {
                ffStrbufDestroy(&device->name);
                ffStrbufDestroy(&device->type);
                result->length--;
                continue;
            }

            ffStrbufInitWS(&device->devPath, szDevice);
            device->bytesRead = (uint64_t) diskPerformance.BytesRead.QuadPart;
            device->readCount = (uint64_t) diskPerformance.ReadCount;
            device->bytesWritten = (uint64_t) diskPerformance.BytesWritten.QuadPart;
            device->writeCount = (uint64_t) diskPerformance.WriteCount;
        }
    }

    return NULL;
}

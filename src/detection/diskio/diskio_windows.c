#include "diskio.h"
#include "common/io/io.h"
#include "util/windows/registry.h"
#include "util/windows/unicode.h"

#include <winioctl.h>

const char* ffDiskIOGetIoCounters(FFlist* result, FFDiskIOOptions* options)
{
    wchar_t szDevice[32] = L"\\\\.\\PhysicalDrive";
    wchar_t* pNum = szDevice + strlen("\\\\.\\PhysicalDrive");
    for (uint32_t idev = 0; ; ++idev)
    {
        _ultow(idev, pNum, 10);

        FF_AUTO_CLOSE_FD HANDLE hDevice = CreateFileW(szDevice, FILE_READ_ATTRIBUTES, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        if (hDevice == INVALID_HANDLE_VALUE)
            break;

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
            sizeof(sddBuffer),
            &retSize,
            NULL
        ) || retSize == 0)
            continue;

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

        if (options->namePrefix.length && !ffStrbufStartsWith(&device->name, &options->namePrefix))
        {
            ffStrbufDestroy(&device->name);
            result->length--;
            continue;
        }

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
            continue;
        }

        ffStrbufInitWS(&device->devPath, szDevice);
        ffStrbufInit(&device->serial);
        if (sdd->SerialNumberOffset != 0)
        {
            ffStrbufSetS(&device->serial, (const char*) sddBuffer + sdd->SerialNumberOffset);
            ffStrbufTrim(&device->serial, ' ');
        }

        device->removable = !!sdd->RemovableMedia;

        ffStrbufInit(&device->interconnect);
        switch (sdd->BusType)
        {
            case BusTypeUnknown: ffStrbufSetStatic(&device->interconnect, "Unknown"); break;
            case BusTypeScsi: ffStrbufSetStatic(&device->interconnect, "Scsi"); break;
            case BusTypeAtapi: ffStrbufSetStatic(&device->interconnect, "Atapi"); break;
            case BusTypeAta: ffStrbufSetStatic(&device->interconnect, "Ata"); break;
            case BusType1394: ffStrbufSetStatic(&device->interconnect, "1394"); break;
            case BusTypeSsa: ffStrbufSetStatic(&device->interconnect, "Ssa"); break;
            case BusTypeFibre: ffStrbufSetStatic(&device->interconnect, "Fibra"); break;
            case BusTypeUsb: ffStrbufSetStatic(&device->interconnect, "Usb"); break;
            case BusTypeRAID: ffStrbufSetStatic(&device->interconnect, "RAID"); break;
            case BusTypeiScsi: ffStrbufSetStatic(&device->interconnect, "iScsi"); break;
            case BusTypeSas: ffStrbufSetStatic(&device->interconnect, "Sas"); break;
            case BusTypeSata: ffStrbufSetStatic(&device->interconnect, "Sata"); break;
            case BusTypeSd: ffStrbufSetStatic(&device->interconnect, "Sd"); break;
            case BusTypeMmc: ffStrbufSetStatic(&device->interconnect, "Mmc"); break;
            case BusTypeVirtual: ffStrbufSetStatic(&device->interconnect, "Virtual"); break;
            case BusTypeFileBackedVirtual: ffStrbufSetStatic(&device->interconnect, "FileBackedVirtual"); break;
            case BusTypeSpaces: ffStrbufSetStatic(&device->interconnect, "Spaces"); break;
            case BusTypeNvme: ffStrbufSetStatic(&device->interconnect, "Nvme"); break;
            case BusTypeSCM: ffStrbufSetStatic(&device->interconnect, "SCM"); break;
            case BusTypeUfs: ffStrbufSetStatic(&device->interconnect, "Ufs"); break;
            default: ffStrbufSetF(&device->interconnect, "Unknown (%d)", (int) sdd->BusType); break;
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

    return NULL;
}

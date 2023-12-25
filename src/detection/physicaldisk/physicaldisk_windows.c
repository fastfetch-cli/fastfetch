#include "physicaldisk.h"
#include "common/io/io.h"
#include "util/windows/unicode.h"

#include <windows.h>
#include <winioctl.h>

static bool detectPhysicalDisk(const wchar_t* szDevice, FFlist* result, FFPhysicalDiskOptions* options)
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
        sizeof(sddBuffer),
        &retSize,
        NULL
    ) || retSize == 0)
        return true;

    FFPhysicalDiskResult* device = (FFPhysicalDiskResult*) ffListAdd(result);
    device->type = FF_PHYSICALDISK_TYPE_NONE;
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
    ffStrbufInit(&device->serial);
    if (sdd->SerialNumberOffset != 0)
    {
        ffStrbufSetS(&device->serial, (const char*) sddBuffer + sdd->SerialNumberOffset);
        ffStrbufTrim(&device->serial, ' ');
    }

    if (sdd->ProductRevisionOffset != 0)
    {
        ffStrbufSetS(&device->revision, (const char*) sddBuffer + sdd->ProductRevisionOffset);
        ffStrbufTrim(&device->revision, ' ');
    }

    device->type |= sdd->RemovableMedia ? FF_PHYSICALDISK_TYPE_REMOVABLE : FF_PHYSICALDISK_TYPE_FIXED;

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
        device->type |= dspd.IncursSeekPenalty ? FF_PHYSICALDISK_TYPE_HDD : FF_PHYSICALDISK_TYPE_SSD;

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

    GET_MEDIA_TYPES gmt = {};
    if(DeviceIoControl(
        hDevice,
        IOCTL_STORAGE_GET_MEDIA_TYPES_EX,
        NULL,
        0,
        &gmt,
        sizeof(gmt),
        &retSize,
        NULL) && gmt.MediaInfoCount > 0
    )
    {
        __typeof__(gmt.MediaInfo[0].DeviceSpecific.DiskInfo)* diskInfo = &gmt.MediaInfo[0].DeviceSpecific.DiskInfo;
        if (diskInfo->MediaCharacteristics & MEDIA_READ_ONLY)
            device->type |= FF_PHYSICALDISK_TYPE_READONLY;
        else if (diskInfo->MediaCharacteristics & MEDIA_READ_WRITE)
            device->type |= FF_PHYSICALDISK_TYPE_READWRITE;
        if (device->size == 0)
            device->size = (uint64_t) diskInfo->NumberMediaSides * diskInfo->TracksPerCylinder * diskInfo->SectorsPerTrack * diskInfo->BytesPerSector;
    }

    return true;
}

const char* ffDetectPhysicalDisk(FFlist* result, FFPhysicalDiskOptions* options)
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

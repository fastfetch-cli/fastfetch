#include "physicaldisk.h"
#include "common/io.h"
#include "common/windows/unicode.h"

#include <stdalign.h>
#include <windows.h>
#include <winioctl.h>

static bool detectPhysicalDisk(const wchar_t* szDevice, FFlist* result, FFPhysicalDiskOptions* options) {
    FF_AUTO_CLOSE_FD HANDLE hDevice = CreateFileW(szDevice, FILE_READ_ATTRIBUTES, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hDevice == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD retSize;
    FFPhysicalDiskType type = FF_PHYSICALDISK_TYPE_NONE;

    uint64_t size = 0;
    {
        alignas(DISK_GEOMETRY_EX) uint8_t dgeBuffer[4096];
        if (DeviceIoControl(
                hDevice,
                IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,
                NULL,
                0,
                dgeBuffer,
                sizeof(dgeBuffer),
                &retSize,
                NULL)) {
            const DISK_GEOMETRY_EX* dge = (const DISK_GEOMETRY_EX*) dgeBuffer;
            size = (uint64_t) dge->DiskSize.QuadPart;
        }
    }
    if (size == 0) {
        type |= FF_PHYSICALDISK_TYPE_UNKNOWN;
    }

    alignas(STORAGE_DEVICE_DESCRIPTOR) uint8_t sddBuffer[4096];
    if (!DeviceIoControl(
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
            NULL) ||
        retSize == 0) {
        return true;
    }
    const STORAGE_DEVICE_DESCRIPTOR* sdd = (const STORAGE_DEVICE_DESCRIPTOR*) sddBuffer;

    const char* interconnect;
    switch (sdd->BusType) {
        case BusTypeScsi:
            interconnect = "SCSI";
            break;
        case BusTypeAtapi:
            interconnect = "ATAPI";
            break;
        case BusTypeAta:
            interconnect = "ATA";
            break;
        case BusType1394:
            interconnect = "IEEE 1394";
            break;
        case BusTypeSsa:
            interconnect = "SSA";
            break;
        case BusTypeFibre:
            interconnect = "Fibre";
            break;
        case BusTypeUsb:
            interconnect = "USB";
            break;
        case BusTypeRAID:
            interconnect = "RAID";
            break;
        case BusTypeiScsi:
            interconnect = "iSCSI";
            break;
        case BusTypeSas:
            interconnect = "SAS";
            break;
        case BusTypeSata:
            interconnect = "SATA";
            break;
        case BusTypeSd:
            interconnect = "SD";
            break;
        case BusTypeMmc:
            interconnect = "MMC";
            break;
        case BusTypeVirtual:
            interconnect = "Virtual";
            type |= FF_PHYSICALDISK_TYPE_VIRTUAL;
            break;
        case BusTypeFileBackedVirtual:
            interconnect = "File Backed Virtual";
            type |= FF_PHYSICALDISK_TYPE_VIRTUAL;
            break;
        case BusTypeSpaces:
            interconnect = "Storage Spaces";
            type |= FF_PHYSICALDISK_TYPE_VIRTUAL;
            break;
        case BusTypeNvme:
            interconnect = "NVMe";
            break;
        case BusTypeSCM:
            interconnect = "SCM";
            break;
        case BusTypeUfs:
            interconnect = "UFS";
            break;
        case 0x14 /*BusTypeNvmeof*/:
            interconnect = "NVMe-oF";
            break;
        default:
            interconnect = "Unknown";
            break;
    }

    FFPhysicalDiskResult* device = (FFPhysicalDiskResult*) ffListAdd(result);
    ffStrbufInit(&device->serial);
    ffStrbufInit(&device->revision);
    ffStrbufInit(&device->name);
    ffStrbufInit(&device->devPath);
    ffStrbufInitStatic(&device->interconnect, interconnect);
    device->type = type;
    device->size = size;
    device->temperature = FF_PHYSICALDISK_TEMP_UNSET;

    if (sdd->VendorIdOffset != 0) {
        ffStrbufSetS(&device->name, (const char*) sddBuffer + sdd->VendorIdOffset);
        ffStrbufTrim(&device->name, ' ');
    }
    if (sdd->ProductIdOffset != 0) {
        if (device->name.length) {
            ffStrbufAppendC(&device->name, ' ');
        }

        ffStrbufAppendS(&device->name, (const char*) sddBuffer + sdd->ProductIdOffset);
        ffStrbufTrimRight(&device->name, ' ');
    }

    if (!device->name.length) {
        ffStrbufSetWS(&device->name, szDevice);
    }

    if (options->namePrefix.length && !ffStrbufStartsWith(&device->name, &options->namePrefix)) {
        ffStrbufDestroy(&device->name);
        result->length--;
        return true;
    }

    ffStrbufSetWS(&device->devPath, szDevice);
    if (sdd->SerialNumberOffset != 0) {
        ffStrbufSetS(&device->serial, (const char*) sddBuffer + sdd->SerialNumberOffset);
        ffStrbufTrimSpace(&device->serial);
    }

    if (sdd->ProductRevisionOffset != 0) {
        ffStrbufSetS(&device->revision, (const char*) sddBuffer + sdd->ProductRevisionOffset);
        ffStrbufTrimRightSpace(&device->revision);
    }

    device->type |= sdd->RemovableMedia ? FF_PHYSICALDISK_TYPE_REMOVABLE : FF_PHYSICALDISK_TYPE_FIXED;

    {
        alignas(GET_MEDIA_TYPES) uint8_t buffer[4096];
        GET_MEDIA_TYPES* gmt = (GET_MEDIA_TYPES*) buffer;
        if (DeviceIoControl(
                hDevice,
                IOCTL_STORAGE_GET_MEDIA_TYPES_EX,
                NULL,
                0,
                gmt,
                sizeof(buffer),
                &retSize,
                NULL) &&
            gmt->MediaInfoCount > 0) {
            // DiskInfo and RemovableDiskInfo have the same structures. TapeInfo doesn't.
            if (gmt->DeviceType != FILE_DEVICE_TAPE) {
                __auto_type diskInfo = &gmt->MediaInfo[0].DeviceSpecific.DiskInfo;
                if (diskInfo->MediaCharacteristics & MEDIA_READ_ONLY) {
                    device->type |= FF_PHYSICALDISK_TYPE_READONLY;
                } else if (diskInfo->MediaCharacteristics & MEDIA_READ_WRITE) {
                    device->type |= FF_PHYSICALDISK_TYPE_READWRITE;
                }
            } else {
                __auto_type tapeInfo = &gmt->MediaInfo[0].DeviceSpecific.TapeInfo;
                if (tapeInfo->MediaCharacteristics & MEDIA_READ_ONLY) {
                    device->type |= FF_PHYSICALDISK_TYPE_READONLY;
                } else if (tapeInfo->MediaCharacteristics & MEDIA_READ_WRITE) {
                    device->type |= FF_PHYSICALDISK_TYPE_READWRITE;
                }
            }
        }
    }

    if (!(device->type & FF_PHYSICALDISK_TYPE_VIRTUAL)) {
        DEVICE_SEEK_PENALTY_DESCRIPTOR dspd = {};
        if (DeviceIoControl(
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
                NULL) &&
            retSize == sizeof(dspd)) {
            device->type |= dspd.IncursSeekPenalty ? FF_PHYSICALDISK_TYPE_HDD : FF_PHYSICALDISK_TYPE_SSD;
        }

        if (options->temp) {
            STORAGE_TEMPERATURE_DATA_DESCRIPTOR stdd = {};
            if (DeviceIoControl(
                    hDevice,
                    IOCTL_STORAGE_QUERY_PROPERTY,
                    &(STORAGE_PROPERTY_QUERY) {
                        .PropertyId = StorageDeviceTemperatureProperty,
                        .QueryType = PropertyStandardQuery,
                    },
                    sizeof(STORAGE_PROPERTY_QUERY),
                    &stdd,
                    sizeof(stdd),
                    &retSize,
                    NULL) &&
                retSize == sizeof(stdd)) {
                device->temperature = stdd.TemperatureInfo[0].Temperature;
            }
        }
    }

    return true;
}

const char* ffDetectPhysicalDisk(FFlist* result, FFPhysicalDiskOptions* options) {
    {
        wchar_t szPhysicalDrive[32] = L"\\\\.\\PhysicalDrive";
        wchar_t* pNum = szPhysicalDrive + strlen("\\\\.\\PhysicalDrive");
        for (uint32_t idev = 0;; ++idev) {
            _ultow(idev, pNum, 10);

            if (!detectPhysicalDisk(szPhysicalDrive, result, options)) {
                break;
            }
        }
    }

    {
        wchar_t szCdrom[32] = L"\\\\.\\CDROM";
        wchar_t* pNum = szCdrom + strlen("\\\\.\\CDROM");
        for (uint32_t idev = 0;; ++idev) {
            _ultow(idev, pNum, 10);

            if (!detectPhysicalDisk(szCdrom, result, options)) {
                break;
            }
        }
    }

    {
        wchar_t szTape[32] = L"\\\\.\\Tape";
        wchar_t* pNum = szTape + strlen("\\\\.\\Tape");
        for (uint32_t idev = 0;; ++idev) {
            _ultow(idev, pNum, 10);

            if (!detectPhysicalDisk(szTape, result, options)) {
                break;
            }
        }
    }

    return NULL;
}

#include "physicaldisk.h"
#include "common/io.h"
#include "common/windows/unicode.h"
#include "common/mallocHelper.h"
#include "common/debug.h"

#include <stdalign.h>
#include <windows.h>
#include <winioctl.h>
#include <cfgmgr32.h>

static const char* detectPhysicalDisk(const char* physicalType, const wchar_t* szDevice, FFlist* result, FFPhysicalDiskOptions* options) {
    FF_AUTO_CLOSE_FD HANDLE hDevice = CreateFileW(szDevice, FILE_READ_ATTRIBUTES, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hDevice == INVALID_HANDLE_VALUE) {
        return "CreateFileW() failed";
    }

    DWORD retSize;
    FFPhysicalDiskType type = FF_PHYSICALDISK_TYPE_NONE;
    FF_STRBUF_AUTO_DESTROY name = ffStrbufCreate();

    const char* interconnect = NULL;
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
        } else if (DeviceIoControl(
                       hDevice,
                       IOCTL_DISK_GET_DRIVE_GEOMETRY,
                       NULL,
                       0,
                       dgeBuffer,
                       sizeof(dgeBuffer),
                       &retSize,
                       NULL) &&
            retSize >= sizeof(DISK_GEOMETRY)) {
            const DISK_GEOMETRY* dg = (const DISK_GEOMETRY*) dgeBuffer;
            size = (uint64_t) dg->BytesPerSector * dg->SectorsPerTrack * dg->TracksPerCylinder * (uint64_t) dg->Cylinders.QuadPart;
            switch (dg->MediaType) {
                case F3_1Pt44_512:
                case F3_2Pt88_512:
                case F3_20Pt8_512:
                case F3_720_512:
                case F3_120M_512:
                case F3_640_512:
                case F3_1Pt2_512:
                case F3_1Pt23_1024:
                case F3_128Mb_512:
                case F3_230Mb_512:
                case F3_200Mb_512:
                case F3_240M_512:
                case F3_32M_512:
                    ffStrbufSetStatic(&name, "3.5-inch Floppy Disk");
                    break;
                case F5_1Pt2_512:
                case F5_360_512:
                case F5_320_512:
                case F5_320_1024:
                case F5_180_512:
                case F5_160_512:
                case F5_640_512:
                case F5_720_512:
                case F5_1Pt23_1024:
                    ffStrbufSetStatic(&name, "5.25-inch Floppy Disk");
                    break;
                case F8_256_128:
                    ffStrbufSetStatic(&name, "8-inch Floppy Disk");
                    break;
                default:
                    return "Unsupported media type";
            }
            interconnect = "Floppy Controller";
            type |= FF_PHYSICALDISK_TYPE_HDD | FF_PHYSICALDISK_TYPE_REMOVABLE;
        }
    }
    if (size == 0) {
        if (options->hideType & FF_PHYSICALDISK_TYPE_UNKNOWN) {
            return "Skipping unknown disk with size 0";
        }

        type |= FF_PHYSICALDISK_TYPE_UNKNOWN;
    }

    const STORAGE_DEVICE_DESCRIPTOR* sdd = NULL;
    alignas(STORAGE_DEVICE_DESCRIPTOR) uint8_t sddBuffer[4096];
    if (!interconnect) {
        if (DeviceIoControl(
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
            sdd = (const STORAGE_DEVICE_DESCRIPTOR*) sddBuffer;

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

            if (type & FF_PHYSICALDISK_TYPE_VIRTUAL && options->hideType & FF_PHYSICALDISK_TYPE_VIRTUAL) {
                return "Skipping virtual disk";
            }

            if (sdd->VendorIdOffset != 0) {
                ffStrbufSetS(&name, (const char*) sdd + sdd->VendorIdOffset);
                ffStrbufTrim(&name, ' ');
            }
            if (sdd->ProductIdOffset != 0) {
                if (name.length) {
                    ffStrbufAppendC(&name, ' ');
                }

                ffStrbufAppendS(&name, (const char*) sdd + sdd->ProductIdOffset);
                ffStrbufTrimRight(&name, ' ');
            }
        }
        if (!name.length) {
            ffStrbufSetStatic(&name, physicalType);
        }
    }

    if (options->namePrefix.length && !ffStrbufStartsWith(&name, &options->namePrefix)) {
        return "Name prefix mismatch";
    }

    FFPhysicalDiskResult* device = (FFPhysicalDiskResult*) ffListAdd(result);
    ffStrbufInit(&device->serial);
    ffStrbufInit(&device->revision);
    ffStrbufInitMove(&device->name, &name);
    ffStrbufInit(&device->devPath);
    ffStrbufInitStatic(&device->interconnect, interconnect);
    device->type = type;
    device->size = size;
    device->temperature = FF_PHYSICALDISK_TEMP_UNSET;

    ffStrbufSetWS(&device->devPath, szDevice);

    if (sdd) {
        if (sdd->SerialNumberOffset != 0) {
            ffStrbufSetS(&device->serial, (const char*) sdd + sdd->SerialNumberOffset);
            ffStrbufTrimSpace(&device->serial);
        }

        if (sdd->ProductRevisionOffset != 0) {
            ffStrbufSetS(&device->revision, (const char*) sdd + sdd->ProductRevisionOffset);
            ffStrbufTrimRightSpace(&device->revision);
        }

        device->type |= sdd->RemovableMedia ? FF_PHYSICALDISK_TYPE_REMOVABLE : FF_PHYSICALDISK_TYPE_FIXED;
    }

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

    if (!(device->type & FF_PHYSICALDISK_TYPE_VIRTUAL) && !(device->type & FF_PHYSICALDISK_TYPE_HDD)) {
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

    return NULL;
}

static void detectPhysicalDisksByInterfaceClass(const char* type, const GUID* interfaceClassGuid, FFlist* result, FFPhysicalDiskOptions* options) {
    ULONG cchDeviceInterfaces = 0;
    if (CM_Get_Device_Interface_List_SizeW(
            &cchDeviceInterfaces,
            (LPGUID) interfaceClassGuid,
            NULL,
            CM_GET_DEVICE_INTERFACE_LIST_PRESENT) != CR_SUCCESS ||
        cchDeviceInterfaces <= 1) {
        return;
    }

    wchar_t* FF_AUTO_FREE mszDeviceInterfaces = (wchar_t*) malloc(cchDeviceInterfaces * sizeof(wchar_t));
    if (!mszDeviceInterfaces) {
        return;
    }

    if (CM_Get_Device_Interface_ListW(
            (LPGUID) interfaceClassGuid,
            NULL,
            mszDeviceInterfaces,
            cchDeviceInterfaces,
            CM_GET_DEVICE_INTERFACE_LIST_PRESENT) != CR_SUCCESS) {
        return;
    }

    // MULTI_SZ: "str1\0str2\0...\0\0"
    for (const wchar_t* p = mszDeviceInterfaces; *p; p += wcslen(p) + 1) {
        FF_DEBUG("Probing %s: %ls", type, p);
        FF_MAYBE_UNUSED const char* error = detectPhysicalDisk(type, p, result, options);
        if (error == NULL) {
            FF_DEBUG("Detected device \"%s\"", FF_LIST_LAST(FFPhysicalDiskResult, *result)->name.chars);
        } else {
            FF_DEBUG("Failed to detect device %s: %s", type, error);
        }
    }
}

const char* ffDetectPhysicalDisk(FFlist* result, FFPhysicalDiskOptions* options) {
    detectPhysicalDisksByInterfaceClass("Floppy", &GUID_DEVINTERFACE_FLOPPY, result, options);
    detectPhysicalDisksByInterfaceClass("Disk", &GUID_DEVINTERFACE_DISK, result, options);
    detectPhysicalDisksByInterfaceClass("CD-ROM", &GUID_DEVINTERFACE_CDROM, result, options);
    detectPhysicalDisksByInterfaceClass("Tape", &GUID_DEVINTERFACE_TAPE, result, options);
    return NULL;
}

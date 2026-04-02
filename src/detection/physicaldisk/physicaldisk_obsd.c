#include "physicaldisk.h"
#include "common/io.h"
#include "common/sysctl.h"

#include <util.h>
#include <sys/ioctl.h>
#include <sys/disklabel.h>
#include <sys/dkio.h>
#include <sys/scsiio.h>
#include <scsi/scsi_all.h>
#include <fcntl.h>
#include <errno.h>

const char* ffDetectPhysicalDisk(FFlist* result, FFPhysicalDiskOptions* options) {
    FF_STRBUF_AUTO_DESTROY diskNames = ffStrbufCreate();
    const char* error = ffSysctlGetString(CTL_HW, HW_DISKNAMES, &diskNames);
    if (error) {
        return error;
    }

    char* diskName = NULL;
    size_t len = 0;
    while (ffStrbufGetdelim(&diskName, &len, ',', &diskNames)) {
        char* colon = strchr(diskName, ':');
        if (colon) {
            *colon = '\0';
        }

        char* devPath = NULL;
        bool readOnly = false;
        FF_AUTO_CLOSE_FD int f = opendev(diskName, O_RDWR, OPENDEV_PART, &devPath);
        if (f < 0) {
            if (errno == EACCES) {
                return "Permission denied; root required";
            } else if (errno == EBUSY) {
                readOnly = true;
                f = opendev(diskName, O_RDONLY, OPENDEV_PART, &devPath);
            }
        }
        if (f < 0) continue; // Unknown error

        struct disklabel dl;
        if (ioctl(f, DIOCGPDINFO, &dl) < 0) {
            continue;
        }

        bool isVirtual = dl.d_type == DTYPE_VND || dl.d_type == DTYPE_RDROOT;
        bool isUnknown = dl.d_ncylinders == 0;
        FFPhysicalDiskResult* device = (FFPhysicalDiskResult*) ffListAdd(result);
        ffStrbufInitS(&device->name, dl.d_packname);
        ffStrbufInitS(&device->devPath, devPath);
        ffStrbufInit(&device->serial);
        ffStrbufInit(&device->revision);
        ffStrbufInitS(&device->interconnect, dl.d_typename);
        device->type = (!isVirtual ? FF_PHYSICALDISK_TYPE_NONE : FF_PHYSICALDISK_TYPE_VIRTUAL) |
            (!isUnknown ? FF_PHYSICALDISK_TYPE_NONE : FF_PHYSICALDISK_TYPE_UNKNOWN) |
            (!readOnly ? FF_PHYSICALDISK_TYPE_READWRITE : FF_PHYSICALDISK_TYPE_READONLY);
        device->size = DL_GETDSIZE(&dl) * dl.d_secsize;
        device->temperature = FF_PHYSICALDISK_TEMP_UNSET;

        struct scsi_inquiry_data inquiry;
        if (ioctl(f, SCIOCCOMMAND, &(struct scsireq) {
                                       .cmd = { [0] = INQUIRY, [4] = sizeof(inquiry) },
                                       .cmdlen = 6,
                                       .databuf = (caddr_t) &inquiry,
                                       .datalen = sizeof(inquiry),
                                       .timeout = 1000,
                                       .flags = SCCMD_READ,
                                   }) == 0) {
            ffStrbufClear(&device->name);
            ffStrbufAppendNS(&device->name, (uint32_t) ARRAY_SIZE(inquiry.vendor), inquiry.vendor);
            ffStrbufTrimRight(&device->name, ' ');
            ffStrbufAppendC(&device->name, ' ');
            ffStrbufAppendNS(&device->name, (uint32_t) ARRAY_SIZE(inquiry.product), inquiry.product);
            ffStrbufTrimRight(&device->name, ' ');
            ffStrbufSetS(&device->revision, inquiry.revision);
            ffStrbufTrimRight(&device->revision, ' ');
            device->type |= inquiry.dev_qual2 & SID_REMOVABLE ? FF_PHYSICALDISK_TYPE_REMOVABLE : FF_PHYSICALDISK_TYPE_FIXED;
        }

        struct scsi_vpd_serial vpdSerial;
        if (ioctl(f, SCIOCCOMMAND, &(struct scsireq) {
                                       .cmd = { [0] = INQUIRY, [1] = SI_EVPD, [2] = SI_PG_SERIAL, [4] = sizeof(vpdSerial) },
                                       .cmdlen = 6,
                                       .databuf = (caddr_t) &vpdSerial,
                                       .datalen = sizeof(vpdSerial),
                                       .timeout = 1000,
                                       .flags = SCCMD_READ,
                                   }) == 0 && vpdSerial.hdr.page_code == SI_PG_SERIAL) {
            ffStrbufSetNS(&device->serial, vpdSerial.hdr.page_length[1], vpdSerial.serial);
            ffStrbufTrimSpace(&device->serial);
        }
    }

    return NULL;
}

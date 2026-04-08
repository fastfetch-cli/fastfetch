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

        if (options->namePrefix.length && !ffStrbufStartsWith(&(FFstrbuf) {
                                                                  .chars = diskName,
                                                                  .length = (uint32_t) len,
                                                              },
                                              &options->namePrefix)) {
            continue;
        }

        char* devPath = NULL;
        FF_AUTO_CLOSE_FD int f = opendev(diskName, O_RDONLY, OPENDEV_PART, &devPath);
        if (f < 0) {
            if (errno == EACCES) {
                return "Permission denied; root required";
            } else {
                continue; // Unknown error
            }
        }

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
            (!isUnknown ? FF_PHYSICALDISK_TYPE_NONE : FF_PHYSICALDISK_TYPE_UNKNOWN);
        device->size = DL_GETDSIZE(&dl) * dl.d_secsize;
        device->temperature = FF_PHYSICALDISK_TEMP_UNSET;

        struct scsi_inquiry_data inquiry = {};
        struct scsireq req = {
            .cmd = { [0] = INQUIRY, [4] = sizeof(inquiry) },
            .cmdlen = 6,
            .databuf = (caddr_t) &inquiry,
            .datalen = sizeof(inquiry),
            .timeout = 1000,
            .flags = SCCMD_READ,
        };
        if (ioctl(f, SCIOCCOMMAND, &req) == 0 && req.retsts == SCCMD_OK) {
            ffStrbufClear(&device->name);
            ffStrbufAppendNS(&device->name, (uint32_t) ARRAY_SIZE(inquiry.vendor), inquiry.vendor);
            ffStrbufTrimRight(&device->name, '\0');
            ffStrbufTrimRight(&device->name, ' ');
            ffStrbufAppendC(&device->name, ' ');
            ffStrbufAppendNS(&device->name, (uint32_t) ARRAY_SIZE(inquiry.product), inquiry.product);
            ffStrbufTrimRight(&device->name, '\0');
            ffStrbufTrimRight(&device->name, ' ');

            ffStrbufSetNS(&device->revision, (uint32_t) ARRAY_SIZE(inquiry.revision), inquiry.revision);
            ffStrbufTrimRight(&device->revision, '\0');
            ffStrbufTrimRight(&device->revision, ' ');
            device->type |= inquiry.dev_qual2 & SID_REMOVABLE ? FF_PHYSICALDISK_TYPE_REMOVABLE : FF_PHYSICALDISK_TYPE_FIXED;
        }

        struct scsi_vpd_serial evpd = {};
        req = (struct scsireq) {
            .cmd = { [0] = INQUIRY, [1] = SI_EVPD, [2] = SI_PG_SERIAL, [4] = sizeof(evpd) },
            .cmdlen = 6,
            .databuf = (caddr_t) &evpd,
            .datalen = sizeof(evpd),
            .timeout = 1000,
            .flags = SCCMD_READ,
        };
        if (ioctl(f, SCIOCCOMMAND, &req) == 0 && req.retsts == SCCMD_OK && evpd.hdr.page_code == SI_PG_SERIAL) {
            for (uint8_t i = 0; i < evpd.hdr.page_length[1]; ++i) {
                ffStrbufAppendF(&device->serial, "%02X", evpd.serial[i]);
            }
            ffStrbufTrimSpace(&device->serial);
        }

        struct scsi_mode_header mode = {};
        req = (scsireq_t) {
            .cmd = { [0] = MODE_SENSE, [2] = SMS_PAGE_CODE, [4] = sizeof(mode) },
            .cmdlen = 6,
            .databuf = (caddr_t) &mode,
            .datalen = sizeof(mode),
            .timeout = 1000,
            .flags = SCCMD_READ,
        };
        if (ioctl(f, SCIOCCOMMAND, &req) == 0 && req.retsts == SCCMD_OK && mode.data_length > 0) {
            device->type |= mode.dev_spec & 0x80 ? FF_PHYSICALDISK_TYPE_READONLY : FF_PHYSICALDISK_TYPE_READWRITE;
        }
    }

    return NULL;
}

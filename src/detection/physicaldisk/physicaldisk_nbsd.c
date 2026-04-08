#define DKTYPENAMES
#include "physicaldisk.h"
#include "common/io.h"
#include "common/sysctl.h"

#include <util.h>
#include <prop/proplib.h>
#include <sys/ioctl.h>
#include <sys/disklabel.h>
#include <sys/dkio.h>
#include <sys/scsiio.h>
#include <dev/scsipi/scsipi_all.h>
#include <dev/scsipi/scsi_spc.h>
#include <fcntl.h>
#include <errno.h>

const char* ffDetectPhysicalDisk(FFlist* result, FFPhysicalDiskOptions* options) {
    FF_STRBUF_AUTO_DESTROY diskNames = ffStrbufCreate();
    const char* error = ffSysctlGetString("hw.disknames", &diskNames);
    if (error) {
        return error;
    }

    char* diskName = NULL;
    size_t len = 0;
    while (ffStrbufGetdelim(&diskName, &len, ' ', &diskNames)) {
        if (options->namePrefix.length && !ffStrbufStartsWith(&(FFstrbuf) {
                                                                  .chars = diskName,
                                                                  .length = (uint32_t) len,
                                                              },
                                              &options->namePrefix)) {
            continue;
        }

        char devPath[256];
        FF_AUTO_CLOSE_FD int f = opendisk(diskName, O_RDONLY, devPath, ARRAY_SIZE(devPath), 0);
        if (f < 0) {
            if (errno == EACCES) {
                return "Permission denied; root required";
            } else {
                continue;
            }
        }

        struct disklabel dl;
        if (ioctl(f, DIOCGDINFO, &dl) < 0) {
            continue;
        }

        unsigned long sectorsPerUnit, sectorSize;

        const char* devType = NULL;

        prop_dictionary_t dict = NULL;
        if (prop_dictionary_recv_ioctl(f, DIOCGDISKINFO, &dict) == 0) {
            prop_dictionary_get_string(dict, "type", &devType);

            prop_dictionary_t geometry;
            if (prop_dictionary_get_dict(dict, "geometry", &geometry)) {
                prop_dictionary_get_ulong(geometry, "sectors-per-unit", &sectorsPerUnit);
                prop_dictionary_get_ulong(geometry, "sector-size", &sectorSize);
            } else {
                sectorsPerUnit = dl.d_secperunit;
                sectorSize = dl.d_secsize;
            }
        } else {
            sectorsPerUnit = dl.d_secperunit;
            sectorSize = dl.d_secsize;
        }

        bool isVirtual = false;
        switch (dl.d_type) {
            case DKTYPE_VND:
            case DKTYPE_LD:
            case DKTYPE_RAID:
            case DKTYPE_CGD:
            case DKTYPE_VINUM:
            case DKTYPE_DM:
            case DKTYPE_RUMPD:
            case DKTYPE_MD:
                isVirtual = true;
        }

        bool isUnknown = sectorsPerUnit == 0;
        FFPhysicalDiskResult* device = (FFPhysicalDiskResult*) ffListAdd(result);
        ffStrbufInitS(&device->name, devType ?: dl.d_packname);
        ffStrbufInitS(&device->devPath, devPath);
        ffStrbufInit(&device->serial);
        ffStrbufInit(&device->revision);
        ffStrbufInitS(&device->interconnect, dktypenames[dl.d_type]);
        device->type = (!isVirtual ? FF_PHYSICALDISK_TYPE_NONE : FF_PHYSICALDISK_TYPE_VIRTUAL) |
            (!isUnknown ? FF_PHYSICALDISK_TYPE_NONE : FF_PHYSICALDISK_TYPE_UNKNOWN) |
            (dl.d_flags & D_REMOVABLE ? FF_PHYSICALDISK_TYPE_REMOVABLE : FF_PHYSICALDISK_TYPE_FIXED);
        device->size = sectorsPerUnit * sectorSize;
        device->temperature = FF_PHYSICALDISK_TEMP_UNSET;

        if (dict) {
            prop_object_release(dict);
            dict = NULL;
        }

        struct scsipi_inquiry_data inquiry = {};
        scsireq_t req = {
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
            ffStrbufTrimRight(&device->name, ',');
            ffStrbufAppendC(&device->name, ' ');
            ffStrbufAppendNS(&device->name, (uint32_t) ARRAY_SIZE(inquiry.product), inquiry.product);
            ffStrbufTrimRight(&device->name, '\0');
            ffStrbufTrimRight(&device->name, ' ');

            ffStrbufSetNS(&device->revision, (uint32_t) ARRAY_SIZE(inquiry.revision), inquiry.revision);
            ffStrbufTrimRight(&device->name, '\0');
            ffStrbufTrimRight(&device->revision, ' ');
        } else {
            continue;
        }

#ifdef SINQ_EVPD // Available since NetBSD 11.0
        struct {
            struct scsipi_inquiry_evpd_header header;
            struct scsipi_inquiry_evpd_serial body;
        } evpd = {};
        req = (scsireq_t) {
            .cmd = { [0] = INQUIRY, [1] = SINQ_EVPD, [2] = SINQ_VPD_UNIT_SERIAL, [4] = sizeof(evpd) },
            .cmdlen = 6,
            .databuf = (caddr_t) &evpd,
            .datalen = sizeof(evpd),
            .timeout = 1000,
            .flags = SCCMD_READ,
        };
        if (ioctl(f, SCIOCCOMMAND, &req) == 0 && req.retsts == SCCMD_OK && evpd.header.pagecode == SINQ_VPD_UNIT_SERIAL) {
            for (uint8_t i = 0; i < evpd.header.length[1]; ++i) {
                ffStrbufAppendF(&device->serial, "%02X", evpd.body.serial_number[i]);
            }
            ffStrbufTrimRight(&device->serial, ' ');
        }
#endif

        struct scsi_mode_parameter_header_6 mode = {};
        req = (scsireq_t) {
            .cmd = { [0] = SCSI_MODE_SENSE_6, [2] = SMS_PAGE_MASK, [4] = sizeof(mode) },
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

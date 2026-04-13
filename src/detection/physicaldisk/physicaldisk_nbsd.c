#define DKTYPENAMES
#include "physicaldisk.h"
#include "common/io.h"
#include "common/sysctl.h"
#include "common/debug.h"

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

static inline const char* retstsToStr(uint8_t retsts) {
    switch (retsts) {
        case SCCMD_OK: return "OK";
        case SCCMD_TIMEOUT: return "TIMEOUT";
        case SCCMD_BUSY: return "BUSY";
        case SCCMD_SENSE: return "SENSE";
        case SCCMD_UNKNOWN: return "UNKNOWN";
        default: return "?";
    }
}

#ifndef NDEBUG
static inline const char* senseKeyToStr(uint8_t key) {
    switch (key) {
        case 0x0: return "No Sense";
        case 0x1: return "Recovered Error";
        case 0x2: return "Not Ready";
        case 0x3: return "Medium Error";
        case 0x4: return "Hardware Error";
        case 0x5: return "Illegal Request";
        case 0x6: return "Unit Attention";
        case 0x7: return "Data Protect";
        case 0x8: return "Blank Check";
        case 0x9: return "Vendor Specific";
        case 0xA: return "Copy Aborted";
        case 0xB: return "Aborted Command";
        case 0xC: return "Equal";
        case 0xD: return "Volume Overflow";
        case 0xE: return "Miscompare";
        case 0xF: return "Completed";
        default: return "Unknown";
    }
}

static void logScsiSense(const char* diskName, const char* operation, const scsireq_t* req) {
    uint8_t responseCode = req->sense[0] & 0x7F;
    uint8_t senseKey;
    uint8_t asc;
    uint8_t ascq;

    if (responseCode == 0x72 || responseCode == 0x73) {
        senseKey = req->sense[1] & 0x0F;
        asc = req->sense[2];
        ascq = req->sense[3];
    } else {
        senseKey = req->sense[2] & 0x0F;
        asc = req->sense[12];
        ascq = req->sense[13];
    }

    FF_STRBUF_AUTO_DESTROY rawSense = ffStrbufCreate();
    for (size_t i = 0; i < req->senselen_used; ++i) {
        if (i)
            ffStrbufAppendC(&rawSense, ' ');
        ffStrbufAppendF(&rawSense, "%02X", req->sense[i]);
    }

    FF_DEBUG(
        "%s for %s reported SENSE: response=0x%02X, key=0x%X (%s), ASC=0x%02X, ASCQ=0x%02X, raw=[%s]",
        operation,
        diskName,
        responseCode,
        senseKey,
        senseKeyToStr(senseKey),
        asc,
        ascq,
        rawSense.length ? rawSense.chars : "empty"
    );
}
#else
#define logScsiSense(...) ((void)0)
#endif

const char* ffDetectPhysicalDisk(FFlist* result, FFPhysicalDiskOptions* options) {
    FF_STRBUF_AUTO_DESTROY diskNames = ffStrbufCreate();
    FF_DEBUG("Querying disk names via sysctl hw.disknames");
    const char* error = ffSysctlGetString("hw.disknames", &diskNames);
    if (error) {
        FF_DEBUG("ffSysctlGetString(hw.disknames) failed: %s", error);
        return error;
    }

    FF_DEBUG("Disk names: %s", diskNames.chars);

    char* diskName = NULL;
    size_t len = 0;
    while (ffStrbufGetdelim(&diskName, &len, ' ', &diskNames)) {
        FF_DEBUG("Probing disk: %s", diskName);

        if (options->namePrefix.length && !ffStrbufStartsWith(&(FFstrbuf) {
                                                                  .chars = diskName,
                                                                  .length = (uint32_t) len,
                                                              },
                                              &options->namePrefix)) {
            FF_DEBUG("Skipping %s due to namePrefix filter", diskName);
            continue;
        }

        char devPath[256];
        FF_AUTO_CLOSE_FD int f = opendisk(diskName, O_RDONLY, devPath, ARRAY_SIZE(devPath), 0);
        if (f < 0) {
            if (errno == EACCES) {
                FF_DEBUG("opendisk(%s) failed: permission denied", diskName);
                return "Permission denied; root required";
            } else {
                FF_DEBUG("opendisk(%s) failed: %s", diskName, strerror(errno));
                continue;
            }
        }

        FF_DEBUG("Opened %s as %s", diskName, devPath);

        struct disklabel dl;
        if (ioctl(f, DIOCGDINFO, &dl) < 0) {
            FF_DEBUG("ioctl(DIOCGDINFO) failed for %s: %s", diskName, strerror(errno));
            continue;
        }

        unsigned long sectorsPerUnit, sectorSize;

        const char* devType = NULL;

        prop_dictionary_t dict = NULL;
        if (prop_dictionary_recv_ioctl(f, DIOCGDISKINFO, &dict) == 0) {
            FF_DEBUG("DIOCGDISKINFO succeeded for %s", diskName);
            prop_dictionary_get_string(dict, "type", &devType);

            prop_dictionary_t geometry;
            if (prop_dictionary_get_dict(dict, "geometry", &geometry)) {
                prop_dictionary_get_ulong(geometry, "sectors-per-unit", &sectorsPerUnit);
                prop_dictionary_get_ulong(geometry, "sector-size", &sectorSize);
            } else {
                FF_DEBUG("No geometry in diskinfo for %s, falling back to disklabel", diskName);
                sectorsPerUnit = dl.d_secperunit;
                sectorSize = dl.d_secsize;
            }
        } else {
            FF_DEBUG("DIOCGDISKINFO failed for %s, falling back to disklabel", diskName);
            sectorsPerUnit = dl.d_secperunit;
            sectorSize = dl.d_secsize;
        }

        FFPhysicalDiskType type = dl.d_flags & D_REMOVABLE ? FF_PHYSICALDISK_TYPE_REMOVABLE : FF_PHYSICALDISK_TYPE_FIXED;

        switch (dl.d_type) {
            case DKTYPE_VND:
            case DKTYPE_LD:
            case DKTYPE_RAID:
            case DKTYPE_CGD:
            case DKTYPE_VINUM:
            case DKTYPE_DM:
            case DKTYPE_RUMPD:
            case DKTYPE_MD: {
                if (options->hideType & FF_PHYSICALDISK_TYPE_VIRTUAL) {
                    FF_DEBUG("Skipping virtual disk %s due to hideType", diskName);
                    continue;
                }

                type |= FF_PHYSICALDISK_TYPE_VIRTUAL;
                break;
            }
        }

        uint64_t size = sectorsPerUnit * sectorSize;
        if (size == 0) {
            if (options->hideType & FF_PHYSICALDISK_TYPE_UNUSED) {
                FF_DEBUG("Skipping unused disk %s due to hideType", diskName);
                continue;
            }

            type |= FF_PHYSICALDISK_TYPE_UNUSED;
        }

        FFPhysicalDiskResult* device = (FFPhysicalDiskResult*) ffListAdd(result);
        ffStrbufInitS(&device->name, devType ?: dl.d_packname);
        ffStrbufInitS(&device->devPath, devPath);
        ffStrbufInit(&device->serial);
        ffStrbufInit(&device->revision);
        ffStrbufInitS(&device->interconnect, dktypenames[dl.d_type]);
        device->type = type;
        device->size = size;
        device->temperature = FF_PHYSICALDISK_TEMP_UNSET;
        FF_DEBUG("Added disk entry: name='%s', devPath='%s', interconnect='%s', size=%llu",
            device->name.chars,
            device->devPath.chars,
            device->interconnect.chars,
            (unsigned long long) device->size);

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
        if (ioctl(f, SCIOCCOMMAND, &req) == 0) {
            if (req.retsts == SCCMD_OK) {
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
                FF_DEBUG("SCSI inquiry for %s: name='%s', revision='%s'", diskName, device->name.chars, device->revision.chars);
            } else {
                FF_DEBUG("SCSI inquiry retsts != SCCMD_OK for %s (%d)", diskName, req.retsts);
                continue;
            }
        } else {
            FF_DEBUG("ioctl(SCIOCCOMMAND) failed for SCSI inquiry on %s: %s", diskName, strerror(errno));
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
        if (ioctl(f, SCIOCCOMMAND, &req) == 0) {
            if (req.retsts == SCCMD_OK && evpd.header.pagecode == SINQ_VPD_UNIT_SERIAL) {
                for (uint8_t i = 0; i < evpd.header.length[1]; ++i) {
                    ffStrbufAppendF(&device->serial, "%02X", evpd.body.serial_number[i]);
                }
                ffStrbufTrimRight(&device->serial, ' ');
                FF_DEBUG("SCSI serial for %s: %s", diskName, device->serial.chars);
            } else if (req.retsts == SCCMD_SENSE) {
                logScsiSense(diskName, "SCSI serial page", &req);
            } else {
                FF_DEBUG("SCSI serial page unavailable for %s (retsts=%s, pagecode=%u)", diskName, retstsToStr(req.retsts), (unsigned) evpd.header.pagecode);
            }
        } else {
            FF_DEBUG("ioctl(SCIOCCOMMAND) failed for SCSI serial on %s: %s", diskName, strerror(errno));
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
        if (ioctl(f, SCIOCCOMMAND, &req) == 0) {
            if (req.retsts == SCCMD_OK && mode.data_length > 0) {
                device->type |= mode.dev_spec & 0x80 ? FF_PHYSICALDISK_TYPE_READONLY : FF_PHYSICALDISK_TYPE_READWRITE;
                FF_DEBUG("SCSI mode for %s indicates: %s", diskName, mode.dev_spec & 0x80 ? "readonly" : "readwrite");
            } else if (req.retsts == SCCMD_SENSE) {
                logScsiSense(diskName, "SCSI mode sense", &req);
            } else {
                FF_DEBUG("SCSI mode sense unavailable for %s (retsts=%s, data_length=%u)", diskName, retstsToStr(req.retsts), (unsigned) mode.data_length);
            }
        } else {
            FF_DEBUG("ioctl(SCIOCCOMMAND) failed for SCSI mode sense on %s: %s", diskName, strerror(errno));
        }

        FF_DEBUG("Detected disk '%s' (%s), type=%u", device->name.chars, diskName, (unsigned) device->type);
    }

    return NULL;
}

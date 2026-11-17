#include "physicaldisk.h"
#include "common/debug.h"
#include "common/io.h"
#include "common/properties.h"
#include "common/strutil.h"

#include <ctype.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef FF_HAVE_DBUS
    #include "common/dbus.h"

    #define FF_UDISKS_BUS "org.freedesktop.UDisks2"
    #define FF_UDISKS_NVME_CONTROLLER "org.freedesktop.UDisks2.NVMe.Controller"
    #define FF_UDISKS_DRIVE_ATA "org.freedesktop.UDisks2.Drive.Ata"

static bool hasUDisks(FFDBusData* dbus) {
    DBusMessage* reply = ffDBusGetMethodReply(dbus, "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "NameHasOwner", FF_UDISKS_BUS, nullptr);
    if (reply == nullptr) {
        return false;
    }

    bool result = false;
    DBusMessageIter iter;
    if (dbus->lib->ffdbus_message_iter_init(reply, &iter)) {
        ffDBusGetBool(dbus, &iter, &result);
    }
    dbus->lib->ffdbus_message_unref(reply);
    return result;
}

static FFPhysicalDiskWarning parseCriticalWarning(FFDBusData* dbus, DBusMessage* reply) {
    DBusMessageIter iter;
    if (!dbus->lib->ffdbus_message_iter_init(reply, &iter) || dbus->lib->ffdbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_VARIANT) {
        return FF_PHYSICALDISK_WARNING_UNSET;
    }

    DBusMessageIter arrayIter;
    dbus->lib->ffdbus_message_iter_recurse(&iter, &arrayIter);
    if (dbus->lib->ffdbus_message_iter_get_arg_type(&arrayIter) != DBUS_TYPE_ARRAY) {
        return FF_PHYSICALDISK_WARNING_UNSET;
    }

    FFPhysicalDiskWarning result = FF_PHYSICALDISK_WARNING_NONE;
    DBusMessageIter stringIter;
    dbus->lib->ffdbus_message_iter_recurse(&arrayIter, &stringIter);
    while (dbus->lib->ffdbus_message_iter_get_arg_type(&stringIter) == DBUS_TYPE_STRING) {
        const char* value;
        dbus->lib->ffdbus_message_iter_get_basic(&stringIter, &value);

        if (ffStrEquals(value, "spare")) {
            result |= FF_PHYSICALDISK_WARNING_SPARE;
        } else if (ffStrEquals(value, "temperature")) {
            result |= FF_PHYSICALDISK_WARNING_TEMPERATURE;
        } else if (ffStrEquals(value, "degraded")) {
            result |= FF_PHYSICALDISK_WARNING_RELIABILITY;
        } else if (ffStrEquals(value, "readonly")) {
            result |= FF_PHYSICALDISK_WARNING_READONLY;
        } else if (ffStrEquals(value, "volatile_mem")) {
            result |= FF_PHYSICALDISK_WARNING_BACKUP;
        } else if (ffStrEquals(value, "pmr_readonly")) {
            result |= FF_PHYSICALDISK_WARNING_PMR_READONLY;
        }

        dbus->lib->ffdbus_message_iter_next(&stringIter);
    }
    return result;
}

static void parseSmartAttributes(FFDBusData* dbus, DBusMessage* reply, FFPhysicalDiskResult* device) {
    DBusMessageIter iter;
    if (!dbus->lib->ffdbus_message_iter_init(reply, &iter) || dbus->lib->ffdbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY) {
        return;
    }

    DBusMessageIter dictIter;
    dbus->lib->ffdbus_message_iter_recurse(&iter, &dictIter);
    while (dbus->lib->ffdbus_message_iter_get_arg_type(&dictIter) == DBUS_TYPE_DICT_ENTRY) {
        DBusMessageIter entryIter;
        dbus->lib->ffdbus_message_iter_recurse(&dictIter, &entryIter);
        if (dbus->lib->ffdbus_message_iter_get_arg_type(&entryIter) != DBUS_TYPE_STRING) {
            break;
        }

        const char* key;
        dbus->lib->ffdbus_message_iter_get_basic(&entryIter, &key);
        dbus->lib->ffdbus_message_iter_next(&entryIter);

        uint64_t value;
        if (ffStrEquals(key, "percent_used")) {
            if (ffDBusGetUint(dbus, &entryIter, &value)) {
                device->percentUsed = (uint8_t) (value > 254 ? 254 : value);
            }
        } else if (ffStrEquals(key, "avail_spare")) {
            if (ffDBusGetUint(dbus, &entryIter, &value)) {
                device->availableSpare = (uint8_t) (value > 100 ? 100 : value);
            }
        }

        dbus->lib->ffdbus_message_iter_next(&dictIter);
    }
}

static void detectNvmeHealth(FFDBusData* dbus, const char* drivePath, FFPhysicalDiskResult* device) {
    uint64_t smartUpdated = 0;
    if (!ffDBusGetPropertyUint(dbus, FF_UDISKS_BUS, drivePath, FF_UDISKS_NVME_CONTROLLER, "SmartUpdated", &smartUpdated)) {
        FF_DEBUG("%s has no NVMe controller interface", drivePath);
        return;
    }
    if (smartUpdated == 0) {
        FF_DEBUG("udisks has not collected SMART data of %s yet", drivePath);
        return;
    }

    DBusMessage* reply = ffDBusGetProperty(dbus, FF_UDISKS_BUS, drivePath, FF_UDISKS_NVME_CONTROLLER, "SmartCriticalWarning");
    if (reply == nullptr) {
        FF_DEBUG("Failed to get SmartCriticalWarning of %s", drivePath);
        return;
    }
    device->criticalWarning = parseCriticalWarning(dbus, reply);
    dbus->lib->ffdbus_message_unref(reply);

    reply = ffDBusGetMethodReplyEmptyDict(dbus, FF_UDISKS_BUS, drivePath, FF_UDISKS_NVME_CONTROLLER, "SmartGetAttributes");
    if (reply == nullptr) {
        FF_DEBUG("SmartGetAttributes failed for %s", drivePath);
        return;
    }
    parseSmartAttributes(dbus, reply, device);
    dbus->lib->ffdbus_message_unref(reply);
}

static void detectAtaHealth(FFDBusData* dbus, const char* drivePath, FFPhysicalDiskResult* device) {
    DBusMessage* reply = ffDBusGetAllProperties(dbus, FF_UDISKS_BUS, drivePath, FF_UDISKS_DRIVE_ATA);
    if (reply == nullptr) {
        FF_DEBUG("%s has no ATA interface", drivePath);
        return;
    }

    bool smartFailing = false;
    uint64_t smartUpdated = 0;
    DBusMessageIter iter;
    if (dbus->lib->ffdbus_message_iter_init(reply, &iter) && dbus->lib->ffdbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_ARRAY) {
        DBusMessageIter dictIter;
        dbus->lib->ffdbus_message_iter_recurse(&iter, &dictIter);
        while (dbus->lib->ffdbus_message_iter_get_arg_type(&dictIter) == DBUS_TYPE_DICT_ENTRY) {
            DBusMessageIter entryIter;
            dbus->lib->ffdbus_message_iter_recurse(&dictIter, &entryIter);
            if (dbus->lib->ffdbus_message_iter_get_arg_type(&entryIter) != DBUS_TYPE_STRING) {
                break;
            }

            const char* key;
            dbus->lib->ffdbus_message_iter_get_basic(&entryIter, &key);
            dbus->lib->ffdbus_message_iter_next(&entryIter);

            if (ffStrEquals(key, "SmartUpdated")) {
                ffDBusGetUint(dbus, &entryIter, &smartUpdated);
            } else if (ffStrEquals(key, "SmartFailing")) {
                ffDBusGetBool(dbus, &entryIter, &smartFailing);
            }

            dbus->lib->ffdbus_message_iter_next(&dictIter);
        }
    }
    dbus->lib->ffdbus_message_unref(reply);

    if (smartUpdated == 0) {
        FF_DEBUG("udisks has not collected SMART data of %s yet", drivePath);
        return;
    }
    if (device->criticalWarning == FF_PHYSICALDISK_WARNING_UNSET) {
        device->criticalWarning = FF_PHYSICALDISK_WARNING_NONE;
    }
    if (smartFailing) {
        device->criticalWarning |= FF_PHYSICALDISK_WARNING_FAILING;
    }
}

static void detectHealth(FFlist* result) {
    FF_DBUS_AUTO_DESTROY_DATA FFDBusData dbus = {};
    const char* error = ffDBusLoadData(DBUS_BUS_SYSTEM, &dbus);
    if (error) {
        FF_DEBUG("%s", error);
        return;
    }
    if (!hasUDisks(&dbus)) {
        FF_DEBUG("udisks2 is not running");
        return;
    }

    FF_LIST_FOR_EACH (FFPhysicalDiskResult, device, *result) {
        if (device->type & FF_PHYSICALDISK_TYPE_VIRTUAL) {
            continue;
        }

        char blockPath[128];
        snprintf(blockPath, ARRAY_SIZE(blockPath), "/org/freedesktop/UDisks2/block_devices/%s", device->devPath.chars + strlen("/dev/"));

        FF_STRBUF_AUTO_DESTROY drivePath = ffStrbufCreate();
        if (!ffDBusGetPropertyString(&dbus, FF_UDISKS_BUS, blockPath, "org.freedesktop.UDisks2.Block", "Drive", &drivePath) || drivePath.length <= 1) {
            FF_DEBUG("Failed to get udisks drive of %s", device->devPath.chars);
            continue;
        }

        if (ffStrbufStartsWithS(&device->devPath, "/dev/nvme")) {
            detectNvmeHealth(&dbus, drivePath.chars, device);
        } else {
            detectAtaHealth(&dbus, drivePath.chars, device);
        }
    }
}
#endif

static double detectNvmeTemp(int devfd) {
    char pathHwmon[] = "hwmon$/temp1_input";

    for (char c = '0'; c <= '9'; c++) // hopefully there's only one digit
    {
        pathHwmon[strlen("hwmon")] = c;
        char buffer[64];
        ssize_t size = ffReadFileDataRelative(devfd, pathHwmon, ARRAY_SIZE(buffer), buffer);
        if (size > 0) {
            buffer[size] = '\0';
            double temp = strtod(buffer, nullptr);
            return temp > 0 && temp < 10000000 /*VMware*/ ? temp / 1000 : FF_PHYSICALDISK_TEMP_UNSET;
        }
    }

    return FF_PHYSICALDISK_TEMP_UNSET;
}

static void parsePhysicalDisk(int dfd, const char* devName, FFPhysicalDiskOptions* options, FFlist* result) {
    uint64_t size = 0;

    {
        char blkSize[32];
        ssize_t fileSize = ffReadFileDataRelative(dfd, "size", ARRAY_SIZE(blkSize) - 1, blkSize);
        if (fileSize > 0) {
            blkSize[fileSize] = 0;
            size = (uint64_t) strtoull(blkSize, nullptr, 10) * 512;
        }
    }

    FFPhysicalDiskType type = FF_PHYSICALDISK_TYPE_NONE;
    if (size == 0) {
        if (options->hideType & FF_PHYSICALDISK_TYPE_UNUSED) {
            return;
        }

        type |= FF_PHYSICALDISK_TYPE_UNUSED;
    }

    int devfd = openat(dfd, "device", O_RDONLY | O_CLOEXEC | O_PATH | O_DIRECTORY);

    if (devfd < 0) {
        if (options->hideType & FF_PHYSICALDISK_TYPE_VIRTUAL) {
            return;
        }

        type |= FF_PHYSICALDISK_TYPE_VIRTUAL;
    }

    FF_STRBUF_AUTO_DESTROY name = ffStrbufCreate();

    if (devfd > 0) {
        if (ffAppendFileBufferRelative(devfd, "vendor", &name)) {
            ffStrbufTrimRightSpace(&name);
            if (name.length > 0) {
                ffStrbufAppendC(&name, ' ');
            }
        }

        ffAppendFileBufferRelative(devfd, "model", &name);
        ffStrbufTrimRightSpace(&name);

        if (name.length == 0) {
            ffStrbufSetS(&name, devName);
        }

        if (ffStrStartsWith(devName, "nvme")) {
            int devid, nsid;
            if (sscanf(devName, "nvme%dn%d", &devid, &nsid) == 2) {
                bool multiNs = nsid > 1;
                if (!multiNs) {
                    char pathSysBlock[32];
                    snprintf(pathSysBlock, ARRAY_SIZE(pathSysBlock), "/dev/nvme%dn2", devid);
                    multiNs = access(pathSysBlock, F_OK) == 0;
                }
                if (multiNs) {
                    // In Asahi Linux, there are multiple namespaces for the same NVMe drive.
                    ffStrbufAppendF(&name, " - %d", nsid);
                }
            }
        }

        if (options->namePrefix.length && !ffStrbufStartsWith(&name, &options->namePrefix)) {
            return;
        }
    } else {
        ffStrbufSetS(&name, devName);
    }

    FFPhysicalDiskResult* device = FF_LIST_ADD(FFPhysicalDiskResult, *result);
    ffStrbufInitMove(&device->name, &name);
    ffStrbufInitF(&device->devPath, "/dev/%s", devName);
    ffStrbufInit(&device->serial);
    ffStrbufInit(&device->revision);
    ffStrbufInit(&device->interconnect);
    device->type = type;
    device->size = size;
    device->temperature = FF_PHYSICALDISK_TEMP_UNSET;
    device->percentUsed = FF_PHYSICALDISK_PERCENT_UNSET;
    device->availableSpare = FF_PHYSICALDISK_PERCENT_UNSET;
    device->criticalWarning = FF_PHYSICALDISK_WARNING_UNSET;

    bool isVirtio = false;
    if (devfd > 0) {
        if (ffStrStartsWith(devName, "nvme")) {
            ffStrbufSetStatic(&device->interconnect, "NVMe");
        } else if (ffStrStartsWith(devName, "mmcblk")) {
            ffStrbufSetStatic(&device->interconnect, "MMC");
        } else {
            char pathSysDeviceLink[64];
            snprintf(pathSysDeviceLink, ARRAY_SIZE(pathSysDeviceLink), "/sys/block/%s/device", devName);
            char pathSysDeviceReal[PATH_MAX];
            if (realpath(pathSysDeviceLink, pathSysDeviceReal)) {
                if (strstr(pathSysDeviceReal, "/usb") != nullptr) {
                    ffStrbufSetStatic(&device->interconnect, "USB");
                } else if (strstr(pathSysDeviceReal, "/ata") != nullptr) {
                    ffStrbufSetStatic(&device->interconnect, "ATA");
                } else if (strstr(pathSysDeviceReal, "/scsi") != nullptr) {
                    ffStrbufSetStatic(&device->interconnect, "SCSI");
                } else if (strstr(pathSysDeviceReal, "/nvme") != nullptr) {
                    ffStrbufSetStatic(&device->interconnect, "NVMe");
                } else if (strstr(pathSysDeviceReal, "/virtio") != nullptr) {
                    ffStrbufSetStatic(&device->interconnect, "VirtIO");
                    isVirtio = true; // VirtIO devices are virtual, but we still want to report it
                } else {
                    if (ffAppendFileBufferRelative(devfd, "transport", &device->interconnect)) {
                        ffStrbufTrimRightSpace(&device->interconnect);
                    }
                }
            }
        }
    } else {
        ffStrbufSetStatic(&device->interconnect, "Virtual");
    }

    if (devfd > 0 && !isVirtio) {
        char isRotationalChar = '1';
        if (ffReadFileDataRelative(dfd, "queue/rotational", 1, &isRotationalChar) > 0) {
            device->type |= isRotationalChar == '1' ? FF_PHYSICALDISK_TYPE_HDD : FF_PHYSICALDISK_TYPE_SSD;
        }

        if (ffReadFileBufferRelative(devfd, "serial", &device->serial)) {
            ffStrbufTrimSpace(&device->serial);
        }

        if (ffReadFileBufferRelative(devfd, "firmware_rev", &device->revision) ||
            ffReadFileBufferRelative(devfd, "rev", &device->revision)) {
            ffStrbufTrimRightSpace(&device->revision);
        }

        if (options->temp) {
            device->temperature = detectNvmeTemp(devfd);
        }
    }

    {
        char removableChar = '0';
        if (ffReadFileDataRelative(dfd, "removable", 1, &removableChar) > 0) {
            device->type |= removableChar == '1' ? FF_PHYSICALDISK_TYPE_REMOVABLE : FF_PHYSICALDISK_TYPE_FIXED;
        }
    }

    {
        char roChar = '0';
        if (ffReadFileDataRelative(dfd, "ro", 1, &roChar) > 0) {
            device->type |= roChar == '1' ? FF_PHYSICALDISK_TYPE_READONLY : FF_PHYSICALDISK_TYPE_READWRITE;
        }
    }
}

const char* ffDetectPhysicalDisk(FFlist* result, FFPhysicalDiskOptions* options) {
    FF_AUTO_CLOSE_DIR DIR* sysBlockDirp = opendir("/sys/block/");
    if (sysBlockDirp == nullptr) {
        FF_DEBUG("opendir(\"/sys/block/\") failed: %s", strerror(errno));
        return "opendir(\"/sys/block/\") == nullptr";
    }

    struct dirent* sysBlockEntry;
    while ((sysBlockEntry = readdir(sysBlockDirp)) != nullptr) {
        const char* const devName = sysBlockEntry->d_name;

        if (devName[0] == '.') {
            continue;
        }

        char pathSysBlock[sizeof("/sys/block/") + sizeof(sysBlockEntry->d_name)];
        snprintf(pathSysBlock, ARRAY_SIZE(pathSysBlock), "/sys/block/%s", devName);

        int dfd = openat(dirfd(sysBlockDirp), devName, O_RDONLY | O_CLOEXEC | O_PATH | O_DIRECTORY);
        if (dfd > 0) {
            parsePhysicalDisk(dfd, devName, options, result);
        }
    }

#ifdef FF_HAVE_DBUS
    if (options->health) {
        detectHealth(result);
    }
#endif

    return nullptr;
}

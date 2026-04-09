#include "physicaldisk.h"
#include "common/stringUtils.h"
#include "sys/scsi/generic/inquiry.h"

#include <libdevinfo.h>
#include <sys/stat.h>

struct FFWalkTreeBundle {
    FFPhysicalDiskOptions* options;
    FFlist* disks;
};

static int walkDevTree(di_node_t node, di_minor_t minor, struct FFWalkTreeBundle* bundle) {
    FFPhysicalDiskOptions* options = bundle->options;
    FFlist* result = bundle->disks;

    if (di_minor_spectype(minor) != S_IFCHR || !ffStrEquals(di_minor_name(minor), "a,raw")) {
        return DI_WALK_CONTINUE;
    }

    char* productId;
    char* vendorId;
    if (di_prop_lookup_strings(DDI_DEV_T_ANY, node, "inquiry-product-id", &productId) > 0 && di_prop_lookup_strings(DDI_DEV_T_ANY, node, "inquiry-vendor-id", &vendorId) > 0) {
        FF_STRBUF_AUTO_DESTROY name = ffStrbufCreateF("%s %s", vendorId, productId);
        if (options->namePrefix.length && !ffStrbufStartsWithIgnCase(&name, &options->namePrefix)) {
            return DI_WALK_CONTINUE;
        }

        int* value;

        FFPhysicalDiskType type = FF_PHYSICALDISK_TYPE_NONE;
        uint64_t size = 0;
        int64_t* nblocks;
        if (di_prop_lookup_int64(DDI_DEV_T_ANY, node, "device-nblocks", &nblocks) > 0) {
            if (*nblocks == 0) {
                if (options->hideType & FF_PHYSICALDISK_TYPE_UNUSED) {
                    return DI_WALK_CONTINUE;
                }

                type |= FF_PHYSICALDISK_TYPE_UNUSED;
            } else if (di_prop_lookup_ints(DDI_DEV_T_ANY, node, "device-blksize", &value) > 0) {
                size = (uint64_t) ((uint64_t) *nblocks * (uint64_t) *value);
            }
        }

        FFPhysicalDiskResult* device = (FFPhysicalDiskResult*) ffListAdd(result);
        ffStrbufInitMove(&device->name, &name);
        ffStrbufInitF(&device->devPath, "/devices%s", di_devfs_path(node));
        ffStrbufInit(&device->serial);
        ffStrbufInit(&device->revision);
        ffStrbufInit(&device->interconnect);
        device->temperature = FF_PHYSICALDISK_TEMP_UNSET;
        device->type = type;
        device->size = size;

        char* buf;
        bool usb = false;
        if (di_prop_lookup_strings(DDI_DEV_T_ANY, node, "inquiry-serial-no", &buf) > 0) {
            ffStrbufSetS(&device->serial, buf);
            ffStrbufTrimSpace(&device->serial);
        } else {
            di_node_t parent = di_parent_node(node);
            if (parent != DI_NODE_NIL && di_prop_lookup_strings(DDI_DEV_T_ANY, parent, "usb-serialno", &buf) > 0) {
                ffStrbufSetS(&device->serial, buf);
                usb = true;
            }
        }
        if (di_prop_lookup_strings(DDI_DEV_T_ANY, node, "inquiry-revision-id", &buf) > 0) {
            ffStrbufSetS(&device->revision, buf);
            ffStrbufTrimRightSpace(&device->revision);
        }

        if (usb) {
            ffStrbufSetStatic(&device->interconnect, "USB");
        } else if (di_prop_lookup_strings(DDI_DEV_T_ANY, node, "class", &buf) > 0) {
            ffStrbufSetS(&device->interconnect, buf);
        } else {
            di_node_t parent = di_parent_node(node);
            if (parent != DI_NODE_NIL && di_prop_lookup_strings(DDI_DEV_T_ANY, parent, "model", &buf) > 0) {
                ffStrbufSetS(&device->interconnect, buf);
            }
        }

        device->type |= di_prop_find(DDI_DEV_T_ANY, node, "removable-media") ? FF_PHYSICALDISK_TYPE_REMOVABLE : FF_PHYSICALDISK_TYPE_FIXED;

        if (di_prop_lookup_ints(DDI_DEV_T_ANY, node, "device-solid-state", &value) > 0) {
            device->type |= *value ? FF_PHYSICALDISK_TYPE_SSD : FF_PHYSICALDISK_TYPE_HDD;
        }
        if (di_prop_lookup_ints(DDI_DEV_T_ANY, node, "inquiry-device-type", &value) > 0) {
            device->type |= *value == DTYPE_DIRECT ? FF_PHYSICALDISK_TYPE_READWRITE : *value == DTYPE_RODIRECT ? FF_PHYSICALDISK_TYPE_READONLY
                                                                                                               : 0;
        }
    }

    return DI_WALK_CONTINUE;
}

const char* ffDetectPhysicalDisk(FFlist* result, FFPhysicalDiskOptions* options) {
    di_node_t rootNode = di_init("/", DINFOCPYALL);
    if (rootNode == DI_NODE_NIL) {
        return "di_init() failed";
    }
    di_walk_minor(rootNode, DDI_NT_BLOCK, DI_WALK_CLDFIRST, &(struct FFWalkTreeBundle) { options, result }, (void*) walkDevTree);
    di_fini(rootNode);

    return NULL;
}

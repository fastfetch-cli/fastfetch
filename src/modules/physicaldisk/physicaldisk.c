#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/temps.h"
#include "common/percent.h"
#include "common/size.h"
#include "detection/physicaldisk/physicaldisk.h"
#include "modules/physicaldisk/physicaldisk.h"

static int sortDevices(const FFPhysicalDiskResult* left, const FFPhysicalDiskResult* right) {
    return ffStrbufComp(&left->name, &right->name);
}

static const char* const warningNames[] = {
    "spare below threshold",
    "temperature",
    "reliability degraded",
    "read-only",
    "backup failed",
    "pmr read-only",
    "failing",
};

static inline double getHealth(const FFPhysicalDiskResult* dev) {
    return dev->percentUsed >= 100 ? 0 : 100 - dev->percentUsed;
}

static void formatKey(const FFPhysicalDiskOptions* options, FFPhysicalDiskResult* dev, uint32_t index, FFstrbuf* key) {
    if (options->moduleArgs.key.length == 0) {
        ffStrbufSetF(key, "%s (%s)", FF_MODULE_GET_DISPLAY_NAME(PhysicalDisk), dev->name.length ? dev->name.chars : dev->devPath.chars);
    } else {
        ffStrbufClear(key);
        FF_PARSE_FORMAT_STRING_CHECKED(key, &options->moduleArgs.key, ((FFformatarg[]) {
                                                                          FF_ARG(index, "index"),
                                                                          FF_ARG(dev->name, "name"),
                                                                          FF_ARG(dev->devPath, "dev-path"),
                                                                          FF_ARG(options->moduleArgs.keyIcon, "icon"),
                                                                          FF_ARG(FF_MODULE_GET_DISPLAY_NAME(PhysicalDisk), "module-name"),
                                                                      }));
    }
}

bool ffPrintPhysicalDisk(FFPhysicalDiskOptions* options) {
    FF_LIST_AUTO_DESTROY result = ffListCreate();
    const char* error = ffDetectPhysicalDisk(&result, options);

    if (error) {
        ffPrintError(FF_MODULE_GET_DISPLAY_NAME(PhysicalDisk), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return false;
    }

    ffListSort(&result, sizeof(FFPhysicalDiskResult), (const void*) sortDevices);

    FFPercentageTypeFlags percentType = options->percent.type == 0 ? instance.config.display.percentType : options->percent.type;

    uint32_t index = 0;
    FF_STRBUF_AUTO_DESTROY key = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();

    FF_LIST_FOR_EACH (FFPhysicalDiskResult, dev, result) {
        formatKey(options, dev, result.length == 1 ? 0 : index + 1, &key);
        ffStrbufClear(&buffer);
        ffSizeAppendNum(dev->size, &buffer);

        const char* physicalType = dev->type & FF_PHYSICALDISK_TYPE_VIRTUAL
            ? "Virtual"
            : dev->type & FF_PHYSICALDISK_TYPE_HDD
            ? "HDD"
            : dev->type & FF_PHYSICALDISK_TYPE_SSD
            ? "SSD"
            : "";
        const char* removableType = dev->type & FF_PHYSICALDISK_TYPE_REMOVABLE
            ? "Removable"
            : dev->type & FF_PHYSICALDISK_TYPE_FIXED
            ? "Fixed"
            : "";
        const char* readOnlyType = dev->type & FF_PHYSICALDISK_TYPE_READONLY
            ? "Read-only"
            : "";

        if (options->moduleArgs.outputFormat.length == 0) {
            ffPrintLogoAndKey(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY);

            if (physicalType[0] || removableType[0] || readOnlyType[0]) {
                ffStrbufAppendS(&buffer, " [");
                if (physicalType[0]) {
                    ffStrbufAppendS(&buffer, physicalType);
                }
                if (removableType[0]) {
                    if (buffer.chars[buffer.length - 1] != '[') {
                        ffStrbufAppendS(&buffer, ", ");
                    }
                    ffStrbufAppendS(&buffer, removableType);
                }
                if (readOnlyType[0]) {
                    if (buffer.chars[buffer.length - 1] != '[') {
                        ffStrbufAppendS(&buffer, ", ");
                    }
                    ffStrbufAppendS(&buffer, readOnlyType);
                }
                ffStrbufAppendC(&buffer, ']');
            }

            if (dev->temperature != FF_PHYSICALDISK_TEMP_UNSET) {
                if (buffer.length > 0) {
                    ffStrbufAppendS(&buffer, " - ");
                }

                ffTempsAppendNum(dev->temperature, &buffer, options->tempConfig, &options->moduleArgs);
            }

            if (dev->percentUsed != FF_PHYSICALDISK_PERCENT_UNSET && (percentType & (FF_PERCENTAGE_TYPE_NUM_BIT | FF_PERCENTAGE_TYPE_BAR_BIT))) {
                ffStrbufAppendS(&buffer, " - Health ");
                bool hasBar = percentType & FF_PERCENTAGE_TYPE_BAR_BIT;
                if (hasBar) {
                    ffPercentAppendBar(&buffer, getHealth(dev), options->percent, &options->moduleArgs);
                }
                if (percentType & FF_PERCENTAGE_TYPE_NUM_BIT) {
                    if (hasBar) {
                        ffStrbufAppendC(&buffer, ' ');
                    }
                    ffPercentAppendNum(&buffer, getHealth(dev), options->percent, hasBar, &options->moduleArgs);
                }
            }

            if (dev->criticalWarning != FF_PHYSICALDISK_WARNING_UNSET) {
                const char* separator = " - Warning: ";
                for (uint32_t i = 0; i < ARRAY_SIZE(warningNames); ++i) {
                    if (dev->criticalWarning & (1 << i)) {
                        ffStrbufAppendS(&buffer, separator);
                        ffStrbufAppendS(&buffer, warningNames[i]);
                        separator = ", ";
                    }
                }
            }
            ffStrbufPutTo(&buffer, stdout);
        } else {
            FF_STRBUF_AUTO_DESTROY tempStr = ffStrbufCreate();
            ffTempsAppendNum(dev->temperature, &tempStr, options->tempConfig, &options->moduleArgs);

            FF_STRBUF_AUTO_DESTROY healthStr = ffStrbufCreate();
            FF_STRBUF_AUTO_DESTROY percentUsedStr = ffStrbufCreate();
            if (dev->percentUsed != FF_PHYSICALDISK_PERCENT_UNSET) {
                ffPercentAppendNum(&healthStr, getHealth(dev), options->percent, false, &options->moduleArgs);
                ffStrbufAppendF(&percentUsedStr, "%u%%", (unsigned) dev->percentUsed);
            }
            FF_STRBUF_AUTO_DESTROY spareStr = ffStrbufCreate();
            if (dev->availableSpare != FF_PHYSICALDISK_PERCENT_UNSET) {
                ffStrbufAppendF(&spareStr, "%u%%", (unsigned) dev->availableSpare);
            }
            FF_LIST_AUTO_DESTROY warnings = ffListCreate();
            if (dev->criticalWarning != FF_PHYSICALDISK_WARNING_UNSET) {
                for (uint32_t i = 0; i < ARRAY_SIZE(warningNames); ++i) {
                    if (dev->criticalWarning & (1 << i)) {
                        ffStrbufInitStatic(FF_LIST_ADD(FFstrbuf, warnings), warningNames[i]);
                    }
                }
            }

            if (dev->type & FF_PHYSICALDISK_TYPE_READWRITE) {
                readOnlyType = "Read-write";
            }
            FF_PRINT_FORMAT_CHECKED(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY, ((FFformatarg[]) {
                                                                                                         FF_ARG(buffer, "size"),
                                                                                                         FF_ARG(dev->name, "name"),
                                                                                                         FF_ARG(dev->interconnect, "interconnect"),
                                                                                                         FF_ARG(dev->devPath, "dev-path"),
                                                                                                         FF_ARG(dev->serial, "serial"),
                                                                                                         FF_ARG(physicalType, "physical-type"),
                                                                                                         FF_ARG(removableType, "removable-type"),
                                                                                                         FF_ARG(readOnlyType, "readonly-type"),
                                                                                                         FF_ARG(dev->revision, "revision"),
                                                                                                         FF_ARG(tempStr, "temperature"),
                                                                                                         FF_ARG(healthStr, "health"),
                                                                                                         FF_ARG(percentUsedStr, "percent-used"),
                                                                                                         FF_ARG(spareStr, "available-spare"),
                                                                                                         FF_ARG(warnings, "critical-warning"),
                                                                                                     }));
        }
        ++index;
    }

    FF_LIST_FOR_EACH (FFPhysicalDiskResult, dev, result) {
        ffStrbufDestroy(&dev->name);
        ffStrbufDestroy(&dev->interconnect);
        ffStrbufDestroy(&dev->devPath);
        ffStrbufDestroy(&dev->serial);
        ffStrbufDestroy(&dev->revision);
    }

    return true;
}

void ffParsePhysicalDiskJsonObject(FFPhysicalDiskOptions* options, yyjson_val* module) {
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach (module, idx, max, key, val) {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs)) {
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "namePrefix")) {
            ffStrbufSetJsonVal(&options->namePrefix, val);
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "hideVirtual")) {
            if (!yyjson_is_bool(val)) {
                ffPrintError(FF_MODULE_GET_DISPLAY_NAME(PhysicalDisk), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "hideVirtual must be a boolean");
            } else {
                if (unsafe_yyjson_is_true(val)) {
                    options->hideType |= FF_PHYSICALDISK_TYPE_VIRTUAL;
                } else {
                    options->hideType &= ~FF_PHYSICALDISK_TYPE_VIRTUAL;
                }
            }
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "hideUnused")) {
            if (!yyjson_is_bool(val)) {
                ffPrintError(FF_MODULE_GET_DISPLAY_NAME(PhysicalDisk), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "hideUnused must be a boolean");
            } else {
                if (unsafe_yyjson_is_true(val)) {
                    options->hideType |= FF_PHYSICALDISK_TYPE_UNUSED;
                } else {
                    options->hideType &= ~FF_PHYSICALDISK_TYPE_UNUSED;
                }
            }
            continue;
        }

        if (ffTempsParseJsonObject(key, val, &options->temp, &options->tempConfig)) {
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "health")) {
            if (!yyjson_is_bool(val)) {
                ffPrintError(FF_MODULE_GET_DISPLAY_NAME(PhysicalDisk), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "health must be a boolean");
            } else {
                options->health = unsafe_yyjson_is_true(val);
            }
            continue;
        }

        if (ffPercentParseJsonObject(key, val, &options->percent)) {
            continue;
        }

        ffPrintError(FF_MODULE_GET_DISPLAY_NAME(PhysicalDisk), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

[[gnu::cold]]
void ffGeneratePhysicalDiskJsonConfig(FFPhysicalDiskOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module) {
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);

    yyjson_mut_obj_add_strbuf(doc, module, "namePrefix", &options->namePrefix);

    ffTempsGenerateJsonConfig(doc, module, options->temp, options->tempConfig);

    yyjson_mut_obj_add_bool(doc, module, "health", options->health);
    ffPercentGenerateJsonConfig(doc, module, options->percent);

    yyjson_mut_obj_add_bool(doc, module, "hideVirtual", !!(options->hideType & FF_PHYSICALDISK_TYPE_VIRTUAL));
    yyjson_mut_obj_add_bool(doc, module, "hideUnused", !!(options->hideType & FF_PHYSICALDISK_TYPE_UNUSED));
}

bool ffGeneratePhysicalDiskJsonResult(FFPhysicalDiskOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module) {
    FF_LIST_AUTO_DESTROY result = ffListCreate();
    const char* error = ffDetectPhysicalDisk(&result, options);

    if (error) {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return false;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");
    FF_LIST_FOR_EACH (FFPhysicalDiskResult, dev, result) {
        yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);
        yyjson_mut_obj_add_strbuf(doc, obj, "name", &dev->name);
        yyjson_mut_obj_add_strbuf(doc, obj, "devPath", &dev->devPath);
        yyjson_mut_obj_add_strbuf(doc, obj, "interconnect", &dev->interconnect);

        if (dev->type & FF_PHYSICALDISK_TYPE_VIRTUAL) {
            yyjson_mut_obj_add_str(doc, obj, "kind", "Virtual");
        } else if (dev->type & FF_PHYSICALDISK_TYPE_HDD) {
            yyjson_mut_obj_add_str(doc, obj, "kind", "HDD");
        } else if (dev->type & FF_PHYSICALDISK_TYPE_SSD) {
            yyjson_mut_obj_add_str(doc, obj, "kind", "SSD");
        } else {
            yyjson_mut_obj_add_null(doc, obj, "kind");
        }

        yyjson_mut_obj_add_uint(doc, obj, "size", dev->size);
        yyjson_mut_obj_add_strbuf(doc, obj, "serial", &dev->serial);

        if (dev->type & FF_PHYSICALDISK_TYPE_REMOVABLE) {
            yyjson_mut_obj_add_bool(doc, obj, "removable", true);
        } else if (dev->type & FF_PHYSICALDISK_TYPE_FIXED) {
            yyjson_mut_obj_add_bool(doc, obj, "removable", false);
        } else {
            yyjson_mut_obj_add_null(doc, obj, "removable");
        }

        if (dev->type & FF_PHYSICALDISK_TYPE_READONLY) {
            yyjson_mut_obj_add_bool(doc, obj, "readOnly", true);
        } else if (dev->type & FF_PHYSICALDISK_TYPE_READWRITE) {
            yyjson_mut_obj_add_bool(doc, obj, "readOnly", false);
        } else {
            yyjson_mut_obj_add_null(doc, obj, "readOnly");
        }

        yyjson_mut_obj_add_bool(doc, obj, "unknown", !!(dev->type & FF_PHYSICALDISK_TYPE_UNUSED));

        yyjson_mut_obj_add_strbuf(doc, obj, "revision", &dev->revision);

        if (dev->temperature != FF_PHYSICALDISK_TEMP_UNSET) {
            yyjson_mut_obj_add_real(doc, obj, "temperature", dev->temperature);
        } else {
            yyjson_mut_obj_add_null(doc, obj, "temperature");
        }

        if (dev->percentUsed != FF_PHYSICALDISK_PERCENT_UNSET) {
            yyjson_mut_obj_add_uint(doc, obj, "percentUsed", dev->percentUsed);
        } else {
            yyjson_mut_obj_add_null(doc, obj, "percentUsed");
        }

        if (dev->availableSpare != FF_PHYSICALDISK_PERCENT_UNSET) {
            yyjson_mut_obj_add_uint(doc, obj, "availableSpare", dev->availableSpare);
        } else {
            yyjson_mut_obj_add_null(doc, obj, "availableSpare");
        }

        if (dev->criticalWarning != FF_PHYSICALDISK_WARNING_UNSET) {
            yyjson_mut_val* warnings = yyjson_mut_obj_add_arr(doc, obj, "criticalWarning");
            for (uint32_t i = 0; i < ARRAY_SIZE(warningNames); ++i) {
                if (dev->criticalWarning & (1 << i)) {
                    yyjson_mut_arr_add_str(doc, warnings, warningNames[i]);
                }
            }
        } else {
            yyjson_mut_obj_add_null(doc, obj, "criticalWarning");
        }
    }

    FF_LIST_FOR_EACH (FFPhysicalDiskResult, dev, result) {
        ffStrbufDestroy(&dev->name);
        ffStrbufDestroy(&dev->interconnect);
        ffStrbufDestroy(&dev->devPath);
        ffStrbufDestroy(&dev->serial);
        ffStrbufDestroy(&dev->revision);
    }

    return true;
}

void ffInitPhysicalDiskOptions(FFPhysicalDiskOptions* options) {
    ffOptionInitModuleArg(&options->moduleArgs, "󰋊");

    ffStrbufInit(&options->namePrefix);
    options->temp = false;
    options->tempConfig = (FFColorRangeConfig) { 50, 70 };
    options->health = false;
    options->percent = (FFPercentageModuleConfig) { 50, 20, 0 };
    options->hideType = FF_PHYSICALDISK_TYPE_UNUSED;
}

void ffDestroyPhysicalDiskOptions(FFPhysicalDiskOptions* options) {
    ffOptionDestroyModuleArg(&options->moduleArgs);
    ffStrbufDestroy(&options->namePrefix);
}

FFModuleBaseInfo ffPhysicalDiskModuleInfo = {
    .name = "PhysicalDisk",
    .description = "Print physical disk information",
    .displayName = {
        .en = "Physical Disk",
        .ar = "القرص الفعلي",
        .cs = "Fyzický disk",
        .de = "Physische Festplatte",
        .es = "Disco físico",
        .fi = "Fyysinen Levy",
        .fr = "Disque physique",
        .gl = "Disco físico",
        .he = "דיסק פיזי",
        .id = "Disk Fisik",
        .it = "Disco fisico",
        .ja = "物理ディスク",
        .ko = "물리 디스크",
        .pl = "Dysk fizyczny",
        .pt = "Disco físico",
        .ru = "Физический диск",
        .tr = "Fiziksel Disk",
        .uk = "Фізичний диск",
        .vi = "Ổ đĩa vật lý",
        .zh_CN = "物理磁盘",
        .zh_TW = "物理磁碟",
    },
    .initOptions = (void*) ffInitPhysicalDiskOptions,
    .destroyOptions = (void*) ffDestroyPhysicalDiskOptions,
    .parseJsonObject = (void*) ffParsePhysicalDiskJsonObject,
    .printModule = (void*) ffPrintPhysicalDisk,
    .generateJsonResult = (void*) ffGeneratePhysicalDiskJsonResult,
    .generateJsonConfig = (void*) ffGeneratePhysicalDiskJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        { "Device size (formatted)", "size" },
        { "Device name *", "name" },
        { "Device interconnect type", "interconnect" },
        { "Device raw file path *", "dev-path" },
        { "Serial number", "serial" },
        { "Device kind (SSD or HDD)", "physical-type" },
        { "Device kind (Removable or Fixed)", "removable-type" },
        { "Device kind (Read-only or Read-write)", "readonly-type" },
        { "Product revision", "revision" },
        { "Device temperature (formatted)", "temperature" },
        { "Device health, 100 - percentage used (formatted)", "health" },
        { "Percentage of rated endurance used", "percent-used" },
        { "Remaining spare capacity", "available-spare" },
        { "Critical warnings (none if healthy)", "critical-warning" },
    })),
    .defaultOrder = 69,
};

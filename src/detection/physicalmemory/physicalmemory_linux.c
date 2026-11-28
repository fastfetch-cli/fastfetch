#include "physicalmemory.h"
#include "common/debug.h"
#include "common/io.h"
#include "common/smbios.h"
#include "common/strutil.h"

// systemd >= 248 decodes SMBIOS Type 16 / 17 into udev properties of the `dmi:id` device
// (see systemd's src/udev/dmi_memory_id/dmi_memory_id.c). The database is world readable,
// while /sys/firmware/dmi/tables/DMI and /dev/mem are root-only, so try it first.
#define FF_UDEV_DMI_ID_DB "/run/udev/data/+dmi:id"
#define FF_UDEV_MEMORY_DEVICE_PREFIX "E:MEMORY_DEVICE_"
#define FF_UDEV_MAX_MEMORY_DEVICES 256

static void ffUdevResultAdd(FFlist* result) {
    *FF_LIST_ADD(FFPhysicalMemoryResult, *result) = (FFPhysicalMemoryResult){
        // Doubles as parse-time state: absent PRESENT / SIZE means "present, unknown
        // size", so it starts true and is only ever cleared by PRESENT=0 or SIZE=0.
        .installed = true,
        .type = ffStrbufCreate(),
        .formFactor = ffStrbufCreate(),
        .locator = ffStrbufCreate(),
        .partNumber = ffStrbufCreate(),
        .vendor = ffStrbufCreate(),
        .serial = ffStrbufCreate(),
    };
}

static void ffUdevResultDestroy(FFPhysicalMemoryResult* device) {
    ffStrbufDestroy(&device->type);
    ffStrbufDestroy(&device->formFactor);
    ffStrbufDestroy(&device->locator);
    ffStrbufDestroy(&device->partNumber);
    ffStrbufDestroy(&device->vendor);
    ffStrbufDestroy(&device->serial);
}

// udev hex-escapes characters that are not allowed in property values (\xHH).
// Also drop the placeholder strings ("Unknown", "0x0000...", "<OUT OF SPEC>", ...).
static void ffUdevSetString(FFstrbuf* strbuf, const char* value) {
    ffStrbufSetS(strbuf, value);
    ffStrbufDecodeHexEscapeSequences(strbuf);
    // Older systemd versions print an empty value, newer ones "<OUT OF SPEC>".
    if (ffStrbufStartsWithS(strbuf, "<OUT OF SPEC>")) {
        ffStrbufClear(strbuf);
    }
    ffCleanUpSmbiosValue(strbuf);
}

// `locator` is the "bank/device" join, but the two properties arrive in no guaranteed
// order. Prepending the bank and appending the device yields the same result either way.
// systemd emits each property at most once per device.
static void ffUdevAppendLocator(FFPhysicalMemoryResult* device, const char* value, bool bank) {
    FF_STRBUF_AUTO_DESTROY part = ffStrbufCreate();
    ffUdevSetString(&part, value);
    if (part.length == 0) {
        return;
    }

    if (device->locator.length > 0) {
        if (bank) {
            ffStrbufPrependC(&device->locator, '/');
        } else {
            ffStrbufAppendC(&device->locator, '/');
        }
    }
    if (bank) {
        ffStrbufPrepend(&device->locator, &part);
    } else {
        ffStrbufAppend(&device->locator, &part);
    }
}

// Splits a line like "E:MEMORY_DEVICE_0_SIZE=4294967296" in place. Returns the field
// name, or nullptr when the line is unrelated or has an out-of-range device index.
// The mere presence of such a line means the udev database describes the memory devices
// completely, whether or not its field is recognized or its value is a placeholder.
static const char* ffUdevParseHeader(char* line, uint32_t* index, const char** value) {
    if (!ffStrStartsWith(line, FF_UDEV_MEMORY_DEVICE_PREFIX)) {
        return nullptr;
    }

    char* eq = strchr(line, '=');
    if (!eq) {
        return nullptr;
    }

    const char* p = line + sizeof(FF_UDEV_MEMORY_DEVICE_PREFIX) - 1;
    char* end = nullptr;
    unsigned long n = strtoul(p, &end, 10);
    if (end == p || *end != '_' || n >= FF_UDEV_MAX_MEMORY_DEVICES) {
        return nullptr;
    }

    *eq = '\0';
    *index = (uint32_t) n;
    *value = eq + 1;
    return end + 1;
}

// The udev database is authoritative: as soon as it contains any MEMORY_DEVICE_*
// property, the set of memory devices it describes is considered complete and the
// caller must not fall back to SMBIOS -- even when every slot is empty, all values
// are placeholders, or showEmptySlots filters everything out of `result`.
static bool ffDetectPhysicalMemoryFromUdev(FFPhysicalMemoryOptions* options, FFlist* result) {
    FF_STRBUF_AUTO_DESTROY db = ffStrbufCreate();
    if (!ffReadFileBuffer(FF_UDEV_DMI_ID_DB, &db)) {
        FF_DEBUG("Failed to read %s", FF_UDEV_DMI_ID_DB);
        return false;
    }

    // The udev index addresses `result` directly: MEMORY_DEVICE_n_* maps to entry
    // `base + n`. Entries are therefore naturally sorted and can be updated in place,
    // without a staging list, a comparator or any result copying. FFstrbuf holds no
    // pointer into itself, so FF_LIST_ADD may relocate existing entries freely.
    // Sparse indices leave placeholder entries behind; they are removed below.
    bool seen[FF_UDEV_MAX_MEMORY_DEVICES] = { false };
    uint16_t totalWidths[FF_UDEV_MAX_MEMORY_DEVICES] = { 0 };
    uint16_t dataWidths[FF_UDEV_MAX_MEMORY_DEVICES] = { 0 };
    const uint32_t base = result->length;
    bool found = false;

    char* line = nullptr;
    size_t len = 0;
    while (ffStrbufGetline(&line, &len, &db)) {
        // `len` must stay intact: the next ffStrbufGetline call advances the cursor by it.
        if (len > 0 && line[len - 1] == '\r') {
            line[len - 1] = '\0';
        }

        uint32_t index;
        const char* value;
        const char* field = ffUdevParseHeader(line, &index, &value);
        if (!field) {
            continue;
        }

        while (base + index >= result->length) {
            ffUdevResultAdd(result);
        }
        found = true;
        seen[index] = true;
        FFPhysicalMemoryResult* device = FF_LIST_GET(FFPhysicalMemoryResult, *result, base + index);

        if (ffStrEquals(field, "SIZE")) {
            device->size = strtoull(value, nullptr, 10);
            if (device->size == 0) {
                device->installed = false;
            }
        } else if (ffStrEquals(field, "PRESENT")) {
            if (strtoul(value, nullptr, 10) == 0) {
                device->installed = false;
            }
        } else if (ffStrEquals(field, "TOTAL_WIDTH")) {
            totalWidths[index] = (uint16_t) strtoul(value, nullptr, 10);
        } else if (ffStrEquals(field, "DATA_WIDTH")) {
            dataWidths[index] = (uint16_t) strtoul(value, nullptr, 10);
        } else if (ffStrEquals(field, "SPEED_MTS")) {
            device->maxSpeed = (uint32_t) strtoul(value, nullptr, 10);
        } else if (ffStrEquals(field, "CONFIGURED_SPEED_MTS")) {
            device->runningSpeed = (uint32_t) strtoul(value, nullptr, 10);
        } else if (ffStrEquals(field, "FORM_FACTOR")) {
            ffUdevSetString(&device->formFactor, value);
        } else if (ffStrEquals(field, "BANK_LOCATOR")) {
            ffUdevAppendLocator(device, value, true);
        } else if (ffStrEquals(field, "LOCATOR")) {
            ffUdevAppendLocator(device, value, false);
        } else if (ffStrEquals(field, "TYPE")) {
            ffUdevSetString(&device->type, value);
        } else if (ffStrEquals(field, "MANUFACTURER")) {
            ffUdevSetString(&device->vendor, value);
        } else if (ffStrEquals(field, "SERIAL_NUMBER")) {
            ffUdevSetString(&device->serial, value);
        } else if (ffStrEquals(field, "PART_NUMBER")) {
            ffUdevSetString(&device->partNumber, value);
        }
    }

    if (!found) {
        FF_DEBUG("No MEMORY_DEVICE_* properties in %s", FF_UDEV_DMI_ID_DB);
        return false;
    }

    // Finalize and drop sparse / empty slots. `i` tracks the current list position
    // while `index` tracks the original udev index, which survives the removals.
    uint32_t i = base;
    for (uint32_t index = 0; i < result->length; ++index) {
        FFPhysicalMemoryResult* device = FF_LIST_GET(FFPhysicalMemoryResult, *result, i);
        if (!seen[index] || (!device->installed && !options->showEmptySlots)) {
            ffUdevResultDestroy(device);
            FF_LIST_REMOVE_AT(FFPhysicalMemoryResult, *result, i);
            continue;
        }

        device->ecc = totalWidths[index] > 0 && dataWidths[index] > 0 &&
            totalWidths[index] > dataWidths[index];

        if (!device->installed) {
            device->maxSpeed = device->runningSpeed = 0;
            ffStrbufSetStatic(&device->type, "Empty");
            ffStrbufClear(&device->vendor);
            ffStrbufClear(&device->serial);
            ffStrbufClear(&device->partNumber);
        } else {
            if (device->type.length == 0) {
                ffStrbufSetStatic(&device->type, "Unknown");
            }
            FFPhysicalMemoryUpdateVendorString(device);
        }
        ++i;
    }
    return true;
}

const char* ffDetectPhysicalMemory(FFPhysicalMemoryOptions* options, FFlist* result) {
    if (ffDetectPhysicalMemoryFromUdev(options, result)) {
        FF_DEBUG("Physical memory detected from %s", FF_UDEV_DMI_ID_DB);
        return nullptr;
    }

    FF_DEBUG("Falling back to reading SMBIOS directly");
    // Implemented in physicalmemory_shared.c. On Linux it is used as a fallback when udev data
    // is unavailable; on other SMBIOS capable platforms it backs ffDetectPhysicalMemory directly.
    const char* ffDetectPhysicalMemoryFromSmbios(FFPhysicalMemoryOptions * options, FFlist * result);
    return ffDetectPhysicalMemoryFromSmbios(options, result);
}

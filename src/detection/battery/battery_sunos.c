#include "battery.h"

#include <kstat.h>

static inline void kstatFreeWrap(kstat_ctl_t** pkc) {
    assert(pkc);
    if (*pkc) {
        kstat_close(*pkc);
    }
}

static bool getUint32(kstat_t* ksp, const char* name, uint32_t* value) {
    kstat_named_t* stat = kstat_data_lookup(ksp, name);
    if (!stat) {
        return false;
    }

    switch (stat->data_type) {
        case KSTAT_DATA_UINT32:
            *value = stat->value.ui32;
            return true;
        case KSTAT_DATA_INT32:
            if (stat->value.i32 >= 0) {
                *value = (uint32_t) stat->value.i32;
                return true;
            }
            break;
#ifdef _INT64_TYPE
        case KSTAT_DATA_UINT64:
            if (stat->value.ui64 <= UINT32_MAX) {
                *value = (uint32_t) stat->value.ui64;
                return true;
            }
            break;
        case KSTAT_DATA_INT64:
            if (stat->value.i64 >= 0 && (uint64_t) stat->value.i64 <= UINT32_MAX) {
                *value = (uint32_t) stat->value.i64;
                return true;
            }
            break;
#endif
        default:
            break;
    }

    return false;
}

const char* ffDetectBattery([[maybe_unused]] FFBatteryOptions* options, FFlist* results) {
    // https://github.com/illumos/illumos-gate/blob/master/usr/src/cmd/powertop/common/battery.c
    [[gnu::cleanup(kstatFreeWrap)]] kstat_ctl_t* kc = kstat_open();
    if (!kc) {
        return "kstat_open() failed";
    }

    // illumos exposes battery information through either of these modules.
    const char* modules[] = { "battery", "acpi_drv" };
    kstat_t* bif = nullptr;
    kstat_t* bst = nullptr;
    for (size_t i = 0; i < ARRAY_SIZE(modules); ++i) {
        bif = kstat_lookup(kc, modules[i], 0, "battery BIF0");
        bst = kstat_lookup(kc, modules[i], 0, "battery BST0");
        if (bif && bst) {
            break;
        }
        bif = nullptr;
        bst = nullptr;
    }

    if (!bif || !bst || kstat_read(kc, bif, nullptr) < 0 || kstat_read(kc, bst, nullptr) < 0) {
        return nullptr;
    }

    uint32_t lastCapacity, remainingCapacity, rate, state;
    if (!getUint32(bif, "bif_last_cap", &lastCapacity) ||
        !getUint32(bst, "bst_rem_cap", &remainingCapacity) ||
        !getUint32(bst, "bst_state", &state)) {
        return "kstat_data_lookup() failed";
    }

    if (lastCapacity == 0 || remainingCapacity == UINT32_MAX) {
        return nullptr;
    }

    FFBatteryResult* battery = FF_LIST_ADD(FFBatteryResult, *results);
    ffStrbufInit(&battery->manufacturer);
    ffStrbufInit(&battery->manufactureDate);
    ffStrbufInit(&battery->modelName);
    ffStrbufInit(&battery->technology);
    ffStrbufInit(&battery->serial);
    battery->status = FF_BATTERY_STATUS_NONE;
    battery->capacity = (double) remainingCapacity * 100.0 / (double) lastCapacity;
    battery->temperature = FF_BATTERY_TEMP_UNSET;
    battery->cycleCount = 0;
    battery->timeRemaining = -1;

    // illumos BST state: bit 0 discharging, bit 1 charging, bit 2 critical.
    if (state & (1u << 0)) {
        battery->status |= FF_BATTERY_STATUS_DISCHARGING;
    }
    if (state & (1u << 1)) {
        battery->status |= FF_BATTERY_STATUS_CHARGING;
    }
    if (state & (1u << 2)) {
        battery->status |= FF_BATTERY_STATUS_CRITICAL;
    }

    if ((state & (1u << 0)) && getUint32(bst, "bst_rate", &rate) && rate > 0 && rate != UINT32_MAX) {
        battery->timeRemaining = (int32_t) (((uint64_t) remainingCapacity * 3600) / rate);
    }

    return nullptr;
}

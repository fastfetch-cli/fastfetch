#include "uptime.h"
#include "common/debug.h"
#include "common/time.h"

#include <kstat.h>

static inline void kstatFreeWrap(kstat_ctl_t** pkc) {
    assert(pkc);
    if (*pkc) {
        kstat_close(*pkc);
    }
}

const char* ffDetectUptime(FFUptimeResult* result) {
    [[gnu::cleanup(kstatFreeWrap)]] kstat_ctl_t* kc = kstat_open();
    if (!kc) {
        FF_DEBUG("kstat_open() failed");
        return "kstat_open() failed";
    }

    kstat_t* ks = kstat_lookup(kc, "unix", 0, "system_misc");
    if (!ks || kstat_read(kc, ks, nullptr) < 0) {
        FF_DEBUG("kstat_lookup(unix:0:system_misc) failed");
        return "kstat_lookup(unix:0:system_misc) failed";
    }

    kstat_named_t* bootTime = (kstat_named_t*) kstat_data_lookup(ks, "boot_time");
    if (!bootTime) {
        FF_DEBUG("kstat_data_lookup(boot_time) failed");
        return "kstat_data_lookup(boot_time) failed";
    }

    result->bootTime = (uint64_t) bootTime->value.ui32 * 1000;
    result->uptime = ffTimeGetNow() - result->bootTime;

    return nullptr;
}

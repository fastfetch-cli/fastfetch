#include "memory.h"
#include <unistd.h>
#include <kstat.h>

static inline void kstatFreeWrap(kstat_ctl_t** pkc) {
    assert(pkc);
    if (*pkc) {
        kstat_close(*pkc);
    }
}

const char* ffDetectMemory(FFMemoryResult* ram) {
    uint64_t pageSize = instance.state.platform.sysinfo.pageSize;

    ram->bytesTotal = (uint64_t) sysconf(_SC_PHYS_PAGES) * pageSize;
    ram->bytesUsed = ram->bytesTotal - (uint64_t) sysconf(_SC_AVPHYS_PAGES) * pageSize;

    FF_A_CLEANUP(kstatFreeWrap) kstat_ctl_t* kc = kstat_open();
    if (kc != NULL) {
        kstat_t* ksp = kstat_lookup(kc, "zfs", -1, "arcstats");
        if (ksp != NULL && kstat_read(kc, ksp, NULL) != -1) {
            kstat_named_t* kn_size = (kstat_named_t*) kstat_data_lookup(ksp, "size");
            kstat_named_t* kn_cmin = (kstat_named_t*) kstat_data_lookup(ksp, "c_min");

            if (kn_size != NULL && kn_cmin != NULL &&
                kn_size->data_type == KSTAT_DATA_UINT64 &&
                kn_cmin->data_type == KSTAT_DATA_UINT64) {
                uint64_t arcSize = kn_size->value.ui64;
                uint64_t arcMin = kn_cmin->value.ui64;

                if (arcSize > arcMin) {
                    uint64_t reclaimableArc = arcSize - arcMin;

                    if (ram->bytesUsed > reclaimableArc) {
                        ram->bytesUsed -= reclaimableArc;
                    }
                }
            }
        }
    }

    return NULL;
}

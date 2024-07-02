#include "memory.h"
#include <kstat.h>

static inline void kstatFreeWrap(kstat_ctl_t** pkc)
{
    assert(pkc);
    if (*pkc)
        kstat_close(*pkc);
}

const char* ffDetectMemory(FFMemoryResult* ram)
{
    __attribute__((__cleanup__(kstatFreeWrap))) kstat_ctl_t* kc = kstat_open();
    if (!kc)
        return "kstat_open() failed";

    kstat_t* ks = kstat_lookup(kc, "unix", -1, "system_pages");
    if (!ks)
        return "kstat_lookup() failed";

    if (kstat_read(kc, ks, NULL) < 0)
        return "kstat_read() failed";

    {
        kstat_named_t* kn = kstat_data_lookup(ks, "pagestotal");
        ram->bytesTotal = kn->value.ui64 * instance.state.platform.sysinfo.pageSize;
    }
    {
        kstat_named_t* kn = kstat_data_lookup(ks, "pagesfree");
        ram->bytesUsed = ram->bytesTotal - kn->value.ui64 * instance.state.platform.sysinfo.pageSize;
    }

    return NULL;
}

#include "cpu.h"
#include <kstat.h>

static inline void kstatFreeWrap(kstat_ctl_t** pkc)
{
    assert(pkc);
    if (*pkc)
        kstat_close(*pkc);
}

const char* ffDetectCPUImpl(FF_MAYBE_UNUSED const FFCPUOptions* options, FFCPUResult* cpu)
{
    __attribute__((__cleanup__(kstatFreeWrap))) kstat_ctl_t* kc = kstat_open();
    if (!kc)
        return "kstat_open() failed";

    kstat_t* ks = kstat_lookup(kc, "cpu_info", -1, NULL);
    if (!ks)
        return "kstat_lookup() failed";

    if (kstat_read(kc, ks, NULL) < 0)
        return "kstat_read() failed";

    {
        kstat_named_t* kn = kstat_data_lookup(ks, "brand");
        ffStrbufSetNS(&cpu->name, KSTAT_NAMED_STR_BUFLEN(kn) - 1, KSTAT_NAMED_STR_PTR(kn));
    }
    {
        kstat_named_t* kn = kstat_data_lookup(ks, "vendor_id");
        ffStrbufSetNS(&cpu->vendor, KSTAT_NAMED_STR_BUFLEN(kn) - 1, KSTAT_NAMED_STR_PTR(kn));
    }
    ffCPUDetectSpeedByCpuid(cpu);
    kstat_named_t* kn = kstat_data_lookup(ks, "clock_MHz");
    if (kn->value.ui32 > cpu->frequencyBase)
        cpu->frequencyBase = kn->value.ui32;

    ks = kstat_lookup(kc, "unix", -1, "system_misc");
    if (ks && kstat_read(kc, ks, NULL) >= 0)
    {
        kstat_named_t* kn = kstat_data_lookup(ks, "ncpus");
        cpu->coresLogical = cpu->coresPhysical = cpu->coresOnline = (uint16_t) kn->value.ui32;
    }

    return NULL;
}

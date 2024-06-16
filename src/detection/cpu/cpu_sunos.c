#include "cpu.h"
#include <kstat.h>

const char* ffDetectCPUImpl(FF_MAYBE_UNUSED const FFCPUOptions* options, FFCPUResult* cpu)
{
    kstat_ctl_t* kc = kstat_open();
    if (!kc)
        return "kstat_open() failed";

    kstat_t* ks = kstat_lookup(kc, "cpu_info", -1, NULL);
    if (!ks)
        return "kstat_lookup() failed";

    if (ks->ks_type != KSTAT_TYPE_NAMED)
        return "Invalid ks_type found";

    if (kstat_read(kc, ks, NULL) < 0)
        return "kstat_read() failed";

    {
        kstat_named_t* kn = kstat_data_lookup(ks, "brand");
        ffStrbufSetNS(&cpu->name, KSTAT_NAMED_STR_BUFLEN(kn), KSTAT_NAMED_STR_PTR(kn));
    }
    {
        kstat_named_t* kn = kstat_data_lookup(ks, "vendor_id");
        ffStrbufSetNS(&cpu->vendor, KSTAT_NAMED_STR_BUFLEN(kn), KSTAT_NAMED_STR_PTR(kn));
    }
    {
        kstat_named_t* kn = kstat_data_lookup(ks, "clock_MHz");
        cpu->frequencyBase = kn->value.i32 / 1000.;
    }

    cpu->coresOnline = 1;
    while ((ks = kstat_lookup(kc, "cpu_info", ks->ks_instance + 1, NULL)))
        cpu->coresOnline++;

    return NULL;
}

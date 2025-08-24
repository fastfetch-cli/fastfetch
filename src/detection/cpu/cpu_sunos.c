#include "cpu.h"
#include "common/processing.h"
#include "util/stringUtils.h"
#include <kstat.h>

static const char* detectCPUTempByKstat(kstat_ctl_t* kc, FFCPUResult* cpu)
{
    const char* possibleModules[] = {"temperature", "cpu_temp", "acpi_thermal", NULL};

    for (int i = 0; possibleModules[i] != NULL; i++) {
        kstat_t* ks = kstat_lookup(kc, possibleModules[i], -1, NULL);
        if (ks && kstat_read(kc, ks, NULL) >= 0) {
            kstat_named_t* kn = kstat_data_lookup(ks, "temperature");
            if (kn) {
                switch (kn->data_type) {
                    case KSTAT_DATA_INT32:
                        cpu->temperature = (float)kn->value.i32;
                        return NULL;
                    case KSTAT_DATA_UINT32:
                        cpu->temperature = (float)kn->value.ui32;
                        return NULL;
                    case KSTAT_DATA_FLOAT:
                        cpu->temperature = kn->value.f;
                        return NULL;
                }
            }
        }
    }

    return "Failed to find CPU temperature using kstat";
}

static const char* detectCPUTempByIpmiTool(FFCPUResult* cpu)
{
    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
    const char* error = ffProcessAppendStdOut(&buffer, (char* const[]){
        "ipmitool",
        "-c",
        "sdr",
        "list",
        NULL
    });

    if (error)
        return error;

    char* line = NULL;
    size_t len = 0;
    while (ffStrbufGetline(&line, &len, &buffer))
    {
        if (sscanf(line, "CPU%*d Temp,%lf,degrees C,ok", &cpu->temperature) == 1)
            return NULL;
    }

    return "ipmitool sdr list failed to find CPU temperature";
}

static inline void kstatFreeWrap(kstat_ctl_t** pkc)
{
    assert(pkc);
    if (*pkc)
        kstat_close(*pkc);
}

static inline uint16_t countTypeId(kstat_ctl_t* kc, const char* type)
{
    uint64_t low = 0, high = 0;
    for (kstat_t* ksp = kc->kc_chain; ksp; ksp = ksp->ks_next)
    {
        if (ffStrStartsWith(ksp->ks_module, "cpu_info"))
        {
            if (kstat_read(kc, ksp, NULL) < 0)
                continue;

            kstat_named_t* stat = kstat_data_lookup(ksp, type);
            if (!stat)
                continue;

            uint32_t id = 0;
            switch (stat->data_type)
            {
                #ifdef _INT64_TYPE
                case KSTAT_DATA_INT64:
                case KSTAT_DATA_UINT64:
                    id = (uint32_t) stat->value.ui64;
                    break;
                #endif
                case KSTAT_DATA_INT32:
                case KSTAT_DATA_UINT32:
                    id = stat->value.ui32;
                    break;
                default:
                    continue;
            }
            if (__builtin_expect(id > 64, false))
                high |= 1ULL << (id - 64);
            else
                low |= 1ULL << id;
        }
    }
    return (uint16_t) (__builtin_popcountll(low) + __builtin_popcountll(high));
}

const char* ffDetectCPUImpl(const FFCPUOptions* options, FFCPUResult* cpu)
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
        if (kn) ffStrbufSetNS(&cpu->name, KSTAT_NAMED_STR_BUFLEN(kn) - 1, KSTAT_NAMED_STR_PTR(kn));
    }
    {
        kstat_named_t* kn = kstat_data_lookup(ks, "vendor_id");
        if (kn) ffStrbufSetNS(&cpu->vendor, KSTAT_NAMED_STR_BUFLEN(kn) - 1, KSTAT_NAMED_STR_PTR(kn));
    }
    ffCPUDetectByCpuid(cpu);
    {
        kstat_named_t* kn = kstat_data_lookup(ks, "clock_MHz");
        if (kn && kn->value.ui32 > cpu->frequencyBase)
            cpu->frequencyBase = kn->value.ui32;
    }

    ks = kstat_lookup(kc, "unix", -1, "system_misc");
    if (ks && kstat_read(kc, ks, NULL) >= 0)
    {
        kstat_named_t* kn = kstat_data_lookup(ks, "ncpus");
        if (kn) cpu->coresLogical = cpu->coresOnline = (uint16_t) kn->value.ui32;
    }

    cpu->packages = countTypeId(kc, "chip_id");
    cpu->coresPhysical = countTypeId(kc, "core_id");

    if (options->temp)
    {
        if (detectCPUTempByKstat(kc, cpu) != NULL)
            detectCPUTempByIpmiTool(cpu);
    }

    return NULL;
}

#include "cpu.h"
#include "common/processing.h"
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
    ffCPUDetectSpeedByCpuid(cpu);
    {
        kstat_named_t* kn = kstat_data_lookup(ks, "clock_MHz");
        if (kn && kn->value.ui32 > cpu->frequencyBase)
            cpu->frequencyBase = kn->value.ui32;
    }

    ks = kstat_lookup(kc, "unix", -1, "system_misc");
    if (ks && kstat_read(kc, ks, NULL) >= 0)
    {
        kstat_named_t* kn = kstat_data_lookup(ks, "ncpus");
        if (kn) cpu->coresLogical = cpu->coresPhysical = cpu->coresOnline = (uint16_t) kn->value.ui32;
    }

    if (options->temp)
    {
        if (detectCPUTempByKstat(kc, cpu) != NULL)
            detectCPUTempByIpmiTool(cpu);
    }

    return NULL;
}

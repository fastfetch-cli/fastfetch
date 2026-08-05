#include "cpu.h"
#include "common/processing.h"
#include "common/strutil.h"
#include <kstat.h>

static const char* detectCPUTempByKstat(const FFCPUOptions* options, kstat_ctl_t* kc, FFCPUResult* cpu) {
    const char* possibleModules[] = { "temperature", "cpu_temp", "acpi_thermal", nullptr };

    if (options->tempSensor.length > 0) {
        possibleModules[0] = options->tempSensor.chars;
        possibleModules[1] = nullptr;
    }

    for (int i = 0; possibleModules[i] != nullptr; i++) {
        kstat_t* ks = kstat_lookup(kc, possibleModules[i], -1, nullptr);
        if (ks && kstat_read(kc, ks, nullptr) >= 0) {
            kstat_named_t* kn = kstat_data_lookup(ks, "temperature");
            if (kn) {
                switch (kn->data_type) {
                    case KSTAT_DATA_INT32:
                        cpu->temperature = (float) kn->value.i32;
                        return nullptr;
                    case KSTAT_DATA_UINT32:
                        cpu->temperature = (float) kn->value.ui32;
                        return nullptr;
                    case KSTAT_DATA_FLOAT:
                        cpu->temperature = kn->value.f;
                        return nullptr;
                }
            }
        }
    }

    return "Failed to find CPU temperature using kstat";
}

static const char* detectCPUTempByIpmiTool(FFCPUResult* cpu) {
    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
    const char* error = ffProcessAppendStdOut(&buffer, (char* const[]) { "ipmitool", "-c", "sdr", "list", nullptr });

    if (error) {
        return error;
    }

    char* line = nullptr;
    size_t len = 0;
    while (ffStrbufGetline(&line, &len, &buffer)) {
        if (sscanf(line, "CPU%*d Temp,%lf,degrees C,ok", &cpu->temperature) == 1) {
            return nullptr;
        }
    }

    return "ipmitool sdr list failed to find CPU temperature";
}

static inline void kstatFreeWrap(kstat_ctl_t** pkc) {
    assert(pkc);
    if (*pkc) {
        kstat_close(*pkc);
    }
}

static inline uint16_t countTypeId(kstat_ctl_t* kc, const char* type) {
    uint64_t low = 0, high = 0;
    for (kstat_t* ksp = kc->kc_chain; ksp; ksp = ksp->ks_next) {
        if (ffStrStartsWith(ksp->ks_module, "cpu_info")) {
            if (kstat_read(kc, ksp, nullptr) < 0) {
                continue;
            }

            kstat_named_t* stat = kstat_data_lookup(ksp, type);
            if (!stat) {
                continue;
            }

            uint32_t id = 0;
            switch (stat->data_type) {
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
            if (__builtin_expect(id > 64, false)) {
                high |= 1ULL << (id - 64);
            } else {
                low |= 1ULL << id;
            }
        }
    }
    return (uint16_t) (__builtin_popcountll(low) + __builtin_popcountll(high));
}

const char* ffDetectCPUImpl(const FFCPUOptions* options, FFCPUResult* cpu) {
    [[gnu::cleanup(kstatFreeWrap)]] kstat_ctl_t* kc = kstat_open();
    if (!kc) {
        return "kstat_open() failed";
    }

    kstat_t* ks = kstat_lookup(kc, "cpu_info", -1, nullptr);
    if (!ks) {
        return "kstat_lookup() failed";
    }

    if (kstat_read(kc, ks, nullptr) < 0) {
        return "kstat_read() failed";
    }

    {
        kstat_named_t* kn = kstat_data_lookup(ks, "brand");
        if (kn) {
            ffStrbufSetNS(&cpu->name, KSTAT_NAMED_STR_BUFLEN(kn) - 1, KSTAT_NAMED_STR_PTR(kn));
        }
    }
    {
        kstat_named_t* kn = kstat_data_lookup(ks, "vendor_id");
        if (kn) {
            ffStrbufSetNS(&cpu->vendor, KSTAT_NAMED_STR_BUFLEN(kn) - 1, KSTAT_NAMED_STR_PTR(kn));
        }
    }
    ffCPUDetectByCpuid(cpu);
    {
        kstat_named_t* kn = kstat_data_lookup(ks, "clock_MHz");
        if (kn && kn->value.ui32 > cpu->frequencyBase) {
            cpu->frequencyBase = kn->value.ui32;
        }
    }

    ks = kstat_lookup(kc, "unix", -1, "system_misc");
    if (ks && kstat_read(kc, ks, nullptr) >= 0) {
        kstat_named_t* kn = kstat_data_lookup(ks, "ncpus");
        if (kn) {
            cpu->coresLogical = cpu->coresOnline = (uint16_t) kn->value.ui32;
        }
    }

    cpu->packages = countTypeId(kc, "chip_id");
    cpu->coresPhysical = countTypeId(kc, "core_id");

    if (options->temp) {
        if (detectCPUTempByKstat(options, kc, cpu) != nullptr) {
            detectCPUTempByIpmiTool(cpu);
        }
    }

    return nullptr;
}

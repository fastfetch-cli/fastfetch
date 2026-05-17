#include "cpu.h"

//#include <cflib.h>
#include <mint/cookie.h>

#define COOKIE__CPU /*(const char *)*/0x5f435055L
#define COOKIE_CT60 /*(const char *)*/0x43543630L

const char* ffDetectCPUImpl(const FFCPUOptions* options, FFCPUResult* cpu) {
    long value = -1;

    cpu->temperature = FF_CPU_TEMP_UNSET;

    if (!Getcookie(COOKIE_CT60, &value)) {
        struct CT60_COOKIE {
            uint16_t trigger_temp, daystop, timestop, speed_fan;
            unsigned long cpu_frequency; // in MHz * 10
            uint16_t beep;
        } *c = (struct CT60_COOKIE *)value;
        ffStrbufSetStatic(&cpu->vendor, "Motorola");
        ffStrbufSetStatic(&cpu->name, "68060");
        cpu->frequencyBase = c->cpu_frequency / 10;

    } else if (!Getcookie(COOKIE__CPU, &value)) {
        ffStrbufSetStatic(&cpu->vendor, "Motorola");
        ffStrbufSetF(&cpu->name, "%d", (68000 + value));
#if defined(__COLDFIRE__) || defined(__mcoldfire__)
    } else {
        ffStrbufSetStatic(&cpu->vendor, "Motorola");
        ffStrbufSetStatic(&cpu->name, "MCF5407");
#endif
    }

    if (value == -1) {
        return "getcookie(_MCH and CT60) failed";
    }

    cpu->packages = (uint16_t)1;
    cpu->coresPhysical = (uint16_t)1;
    cpu->coresLogical = cpu->coresPhysical;
    cpu->coresOnline = (uint16_t)1;

    return NULL;
}

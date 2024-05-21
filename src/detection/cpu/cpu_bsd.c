#include "cpu.h"
#include "common/sysctl.h"
#include "detection/temps/temps_bsd.h"

const char* ffDetectCPUImpl(const FFCPUOptions* options, FFCPUResult* cpu)
{
    if (ffSysctlGetString("hw.model", &cpu->name))
        return "sysctlbyname(hw.model) failed";

    cpu->coresPhysical = (uint16_t) ffSysctlGetInt("hw.ncpu", 1);
    cpu->coresLogical = cpu->coresPhysical;
    cpu->coresOnline = cpu->coresPhysical;

    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
    if (ffSysctlGetString("kern.sched.topology_spec", &buffer) == NULL && buffer.length > 0)
    {
        // <groups>
        // <group level="1" cache-level="3">
        // <cpu count="4" mask="f,0,0,0">0, 1, 2, 3</cpu>
        // <children>
        // <group level="2" cache-level="2">
        // <cpu count="2" mask="3,0,0,0">0, 1</cpu>
        // <flags><flag name="THREAD">THREAD group</flag><flag name="SMT">SMT group</flag></flags>
        // </group>
        // <group level="2" cache-level="2">
        // <cpu count="2" mask="c,0,0,0">2, 3</cpu>
        // <flags><flag name="THREAD">THREAD group</flag><flag name="SMT">SMT group</flag></flags>
        // </group>
        // </children>
        // </group>
        // </groups>
        uint32_t i = 0;
        while (true)
        {
            i = ffStrbufNextIndexS(&buffer, i, "<flag name=\"THREAD\">THREAD group</flag>"); // Find physical core with hyper-threading enabled
            if (i >= buffer.length) break;
            cpu->coresPhysical--;
            i += (uint32_t) strlen("<flag name=\"THREAD\">THREAD group</flag>");
        }
    }

    for (uint16_t i = 0; i < cpu->coresLogical; ++i)
    {
        ffStrbufClear(&buffer);
        char key[32];
        snprintf(key, sizeof(key), "dev.cpu.%u.freq_levels", i);
        if (ffSysctlGetString(key, &buffer) == NULL && buffer.length > 0)
        {
            // MHz/Watts pairs like: 2501/32000 2187/27125 2000/24000
            uint32_t fmax = (uint32_t) strtoul(buffer.chars, NULL, 10);
            uint32_t fmin = fmax;
            uint32_t i = ffStrbufLastIndexC(&buffer, ' ');
            if (i < buffer.length)
                fmin = (uint32_t) strtoul(buffer.chars + i + 1, NULL, 10);
            if (!(cpu->frequencyMin <= fmin)) cpu->frequencyMin = fmin; // Counting for NaN
            if (!(cpu->frequencyMax >= fmax)) cpu->frequencyMax = fmax;

            if (options->showPeCoreCount)
            {
                uint32_t ifreq = 0;
                while (cpu->coreTypes[ifreq].freq != fmax && cpu->coreTypes[ifreq].freq > 0)
                    ++ifreq;
                if (cpu->coreTypes[ifreq].freq == 0)
                    cpu->coreTypes[ifreq].freq = fmax;
                cpu->coreTypes[ifreq].count++;
            }
        }
    }
    cpu->frequencyMin /= 1000;
    cpu->frequencyMax /= 1000;

    int clockRate = ffSysctlGetInt("hw.clockrate", 0);
    cpu->frequencyBase = clockRate <= 0 ? 0.0/0.0 : clockRate / 1000.0;
    cpu->temperature = FF_CPU_TEMP_UNSET;

    if (options->temp)
    {
        if (!ffDetectCpuTemp(&cpu->temperature))
            ffDetectThermalTemp(&cpu->temperature);
    }

    return NULL;
}

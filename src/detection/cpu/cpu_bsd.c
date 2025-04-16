#include "cpu.h"
#include "common/sysctl.h"
#include "util/stringUtils.h"

#include <sys/param.h>
#if __has_include(<sys/cpuset.h>)
    #include <sys/cpuset.h>
    #define FF_HAVE_CPUSET 1
#endif

static const char* detectCpuTemp(double* current)
{
    int temp = ffSysctlGetInt("dev.cpu.0.temperature", -999999);
    if (temp == -999999)
        return "ffSysctlGetInt(\"dev.cpu.0.temperature\") failed";

    // In tenth of degrees Kelvin
    *current = (double) temp / 10 - 273.15;
    return NULL;
}

static const char* detectThermalTemp(double* current)
{
    int temp = ffSysctlGetInt("hw.acpi.thermal.tz0.temperature", -999999);
    if (temp == -999999)
        return "ffSysctlGetInt(\"hw.acpi.thermal.tz0.temperature\") failed";

    // In tenth of degrees Kelvin
    *current = (double) temp / 10 - 273.15;
    return NULL;
}

const char* ffDetectCPUImpl(const FFCPUOptions* options, FFCPUResult* cpu)
{
    if (ffSysctlGetString("hw.model", &cpu->name) != NULL)
        return "sysctlbyname(hw.model) failed";

    cpu->coresLogical = (uint16_t) ffSysctlGetInt("hw.ncpu", 1);
    cpu->coresPhysical = 0;
    cpu->coresOnline = (uint16_t) ffSysctlGetInt("kern.smp.cpus", cpu->coresLogical);

    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
    if (ffSysctlGetString("kern.sched.topology_spec", &buffer) == NULL && buffer.length > 0)
    {
        // <groups>
        //  <group level="1" cache-level="3">
        //   <cpu count="4" mask="f,0,0,0">0, 1, 2, 3</cpu>
        //   <children>
        //    <group level="2" cache-level="2">
        //     <cpu count="2" mask="3,0,0,0">0, 1</cpu>
        //     <flags><flag name="THREAD">THREAD group</flag><flag name="SMT">SMT group</flag></flags>
        //    </group>
        //    <group level="2" cache-level="2">
        //     <cpu count="2" mask="c,0,0,0">2, 3</cpu>
        //     <flags><flag name="THREAD">THREAD group</flag><flag name="SMT">SMT group</flag></flags>
        //    </group>
        //   </children>
        //  </group>
        // </groups>
        char* line = NULL;
        size_t len = 0;
        bool inLvl2 = false, threadGroup = false;
        uint32_t cpuCount = 0;
        while (ffStrbufGetline(&line, &len, &buffer))
        {
            if (!inLvl2)
            {
                if (ffStrStartsWith(line, "   <group level=\"2\""))
                {
                    inLvl2 = true;
                    cpuCount = 0;
                    threadGroup = false;
                }
                else if (ffStrStartsWith(line, " <group level=\"1\""))
                    cpu->packages++;
            }
            else
            {
                if (ffStrEquals(line, "   </group>"))
                {
                    cpu->coresPhysical += threadGroup ? 1 : cpuCount;
                    inLvl2 = false;
                }
                else if (cpuCount == 0 && ffStrStartsWith(line, "    <cpu count=\""))
                    cpuCount = (uint32_t) strtoul(line + strlen("    <cpu count=\""), NULL, 10);
                else if (cpuCount > 0 && ffStrStartsWith(line, "    <flags>"))
                {
                    if (ffStrContains(line, "<flag name=\"THREAD\">THREAD group</flag>"))
                        threadGroup = true;
                }
            }
        }
    }

#if FF_HAVE_CPUSET && (__x86_64__ || __i386__)
    // Bind current process to the first two cores, which is *usually* a performance core
    cpuset_t currentCPU;
    CPU_ZERO(&currentCPU);
    CPU_SET(1, &currentCPU);
    CPU_SET(2, &currentCPU);
    cpuset_setaffinity(CPU_LEVEL_WHICH, CPU_WHICH_TID, -1, sizeof(cpuset_t), &currentCPU);
#endif

    ffCPUDetectSpeedByCpuid(cpu);

    uint32_t clockrate = (uint32_t) ffSysctlGetInt("hw.clockrate", 0);
    if (clockrate > cpu->frequencyBase) cpu->frequencyBase = clockrate;

    for (uint16_t i = 0; i < cpu->coresLogical; ++i)
    {
        ffStrbufClear(&buffer);
        char key[32];
        snprintf(key, sizeof(key), "dev.cpu.%u.freq_levels", i);
        if (ffSysctlGetString(key, &buffer) == NULL)
        {
            if (buffer.length == 0) continue;

            // MHz/Watts pairs like: 2501/32000 2187/27125 2000/24000
            uint32_t fmax = (uint32_t) strtoul(buffer.chars, NULL, 10);
            if (cpu->frequencyMax < fmax) cpu->frequencyMax = fmax;
        }
        else
            break;
    }

    cpu->temperature = FF_CPU_TEMP_UNSET;

    if (options->temp)
    {
        if (detectCpuTemp(&cpu->temperature) != NULL)
            detectThermalTemp(&cpu->temperature);
    }

    return NULL;
}

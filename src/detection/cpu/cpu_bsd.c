#include "cpu.h"
#include "common/sysctl.h"

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

    cpu->coresPhysical = (uint16_t) ffSysctlGetInt("hw.ncpu", 1);
    cpu->coresLogical = cpu->coresPhysical;
    cpu->coresOnline = (uint16_t) ffSysctlGetInt("kern.smp.cpus", cpu->coresLogical);

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
    cpu->temperature = FF_CPU_TEMP_UNSET;

    if (options->temp)
    {
        if (detectCpuTemp(&cpu->temperature) != NULL)
            detectThermalTemp(&cpu->temperature);
    }

    return NULL;
}

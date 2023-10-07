#include "cpu.h"
#include "common/sysctl.h"
#include "detection/temps/temps_apple.h"

static double detectCpuTemp(const FFstrbuf* cpuName)
{
    double result = 0;

    const char* error = NULL;
    if(ffStrbufStartsWithS(cpuName, "Apple M1"))
        error = ffDetectCoreTemps(FF_TEMP_CPU_M1X, &result);
    else if(ffStrbufStartsWithS(cpuName, "Apple M2"))
        error = ffDetectCoreTemps(FF_TEMP_CPU_M2X, &result);
    else // PPC?
        error = ffDetectCoreTemps(FF_TEMP_CPU_X64, &result);

    if(error)
        return FF_CPU_TEMP_UNSET;

    return result;
}

#ifdef __aarch64__
#include "util/apple/cf_helpers.h"

#include <IOKit/IOKitLib.h>

static const char* detectFrequency(FFCPUResult* cpu)
{
    // https://github.com/giampaolo/psutil/pull/2222/files

    io_iterator_t iterator;
    if(IOServiceGetMatchingServices(MACH_PORT_NULL, IOServiceMatching("AppleARMIODevice"), &iterator) != kIOReturnSuccess)
        return "IOServiceGetMatchingServices() failed";

    io_registry_entry_t registryEntry;
    while((registryEntry = IOIteratorNext(iterator)) != 0)
    {
        CFMutableDictionaryRef properties;
        if(IORegistryEntryCreateCFProperties(registryEntry, &properties, kCFAllocatorDefault, kNilOptions) != kIOReturnSuccess)
        {
            IOObjectRelease(registryEntry);
            continue;
        }

        io_name_t name;
        if (IORegistryEntryGetName(registryEntry, name) != KERN_SUCCESS)
            continue;
        if (strcmp(name, "pmgr") != 0)
            continue;

        uint32_t pMin, eMin, aMax, pCoreLength;
        ffCfDictGetData(properties, CFSTR("voltage-states5-sram"), 0, 4, (uint8_t*) &pMin, &pCoreLength); // pCore
        ffCfDictGetData(properties, CFSTR("voltage-states1-sram"), 0, 4, (uint8_t*) &eMin, NULL); // eCore
        cpu->frequencyMin = (pMin < eMin ? pMin : eMin) / (1000.0 * 1000 * 1000);

        if (pCoreLength >= 8)
        {
            ffCfDictGetData(properties, CFSTR("voltage-states5-sram"), pCoreLength - 8, 4, (uint8_t*) &aMax, NULL);
            cpu->frequencyMax = aMax / (1000.0 * 1000 * 1000);
        }
        else
            cpu->frequencyMax = 0.0;

        CFRelease(properties);
        IOObjectRelease(registryEntry);
    }

    IOObjectRelease(iterator);
    return NULL;
}
#else
static const char* detectFrequency(FFCPUResult* cpu)
{
    cpu->frequencyMin = ffSysctlGetInt64("hw.cpufrequency_min", 0) / 1000.0 / 1000.0 / 1000.0;
    cpu->frequencyMax = ffSysctlGetInt64("hw.cpufrequency_max", 0);
    if(cpu->frequencyMax > 0.0)
        cpu->frequencyMax /= 1000.0 * 1000.0 * 1000.0;
    else
    {
        unsigned current = 0;
        size_t size = sizeof(current);
        if (sysctl((int[]){ CTL_HW, HW_CPU_FREQ }, 2, &current, &size, NULL, 0) == 0)
            cpu->frequencyMax = (double) current / 1000.0 / 1000.0 / 1000.0;
    }
    return NULL;
}
#endif

const char* ffDetectCPUImpl(const FFCPUOptions* options, FFCPUResult* cpu)
{
    if (ffSysctlGetString("machdep.cpu.brand_string", &cpu->name) != NULL)
        return "sysctlbyname(machdep.cpu.brand_string) failed";

    ffSysctlGetString("machdep.cpu.vendor", &cpu->vendor);
    if (cpu->vendor.length == 0 && ffStrbufStartsWithS(&cpu->name, "Apple "))
        ffStrbufAppendS(&cpu->vendor, "Apple");

    cpu->coresPhysical = (uint16_t) ffSysctlGetInt("hw.physicalcpu_max", 1);
    if(cpu->coresPhysical == 1)
        cpu->coresPhysical = (uint16_t) ffSysctlGetInt("hw.physicalcpu", 1);

    cpu->coresLogical = (uint16_t) ffSysctlGetInt("hw.logicalcpu_max", 1);
    if(cpu->coresLogical == 1)
        cpu->coresLogical = (uint16_t) ffSysctlGetInt("hw.ncpu", 1);

    cpu->coresOnline = (uint16_t) ffSysctlGetInt("hw.logicalcpu", 1);
    if(cpu->coresOnline == 1)
        cpu->coresOnline = (uint16_t) ffSysctlGetInt("hw.activecpu", 1);

    detectFrequency(cpu);

    cpu->temperature = options->temp ? detectCpuTemp(&cpu->name) : FF_CPU_TEMP_UNSET;

    return NULL;
}

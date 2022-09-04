#include "cpu.h"
#include "common/settings.h"

static double getFrequency(const char* propName)
{
    double herz = (double) ffSettingsGetAppleInt64(propName, 0);
    if(herz <= 0.0)
        return herz;

    herz /= 1000.0; //to KHz
    herz /= 1000.0; //to MHz
    return herz / 1000.0; //to GHz
}

void ffDetectCPUImpl(FFCPUResult* cpu)
{
    ffStrbufInit(&cpu->name);
    ffSettingsGetAppleProperty("machdep.cpu.brand_string", &cpu->name);

    ffStrbufInit(&cpu->vendor);
    ffSettingsGetAppleProperty("machdep.cpu.vendor", &cpu->vendor);

    cpu->coresPhysical = (uint16_t) ffSettingsGetAppleInt("hw.physicalcpu_max", 1);
    if(cpu->coresPhysical == 1)
        cpu->coresPhysical = (uint16_t) ffSettingsGetAppleInt("hw.physicalcpu", 1);

    cpu->coresLogical = (uint16_t) ffSettingsGetAppleInt("hw.logicalcpu_max", 1);
    if(cpu->coresLogical == 1)
        cpu->coresLogical = (uint16_t) ffSettingsGetAppleInt("hw.ncpu", 1);

    cpu->coresOnline = (uint16_t) ffSettingsGetAppleInt("hw.logicalcpu", 1);
    if(cpu->coresOnline == 1)
        cpu->coresOnline = (uint16_t) ffSettingsGetAppleInt("hw.activecpu", 1);

    cpu->frequencyMin = getFrequency("hw.cpufrequency_min");
    cpu->frequencyMax = getFrequency("hw.cpufrequency_max");
    if(cpu->frequencyMax == 0.0)
        cpu->frequencyMax = getFrequency("hw.cpufrequency");

    //TODO find a way to detect this
    cpu->temperature = FF_CPU_TEMP_UNSET;
}

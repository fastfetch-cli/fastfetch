#include "cpu.h"
#include "common/io/io.h"
#include "common/processing.h"
#include "common/properties.h"
#include "detection/temps/temps_linux.h"
#include "util/mallocHelper.h"

#include <sys/sysinfo.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#ifdef __ANDROID__
#include "common/settings.h"

static void detectAndroid(FFCPUResult* cpu)
{
    if (cpu->name.length == 0)
        ffSettingsGetAndroidProperty("ro.soc.model", &cpu->name);
    if (cpu->vendor.length == 0)
    {
        if (!ffSettingsGetAndroidProperty("ro.soc.manufacturer", &cpu->vendor))
            ffSettingsGetAndroidProperty("ro.product.product.manufacturer", &cpu->vendor);
    }
}
#endif

static const char* parseCpuInfo(FFCPUResult* cpu, FFstrbuf* physicalCoresBuffer, FFstrbuf* cpuMHz, FFstrbuf* cpuIsa, FFstrbuf* cpuUarch)
{
    FF_AUTO_CLOSE_FILE FILE* cpuinfo = fopen("/proc/cpuinfo", "r");
    if(cpuinfo == NULL)
        return "fopen(\"/proc/cpuinfo\", \"r\") failed";

    FF_AUTO_FREE char* line = NULL;
    size_t len = 0;

    while(getline(&line, &len, cpuinfo) != -1)
    {
        //Stop after the first CPU
        if(*line == '\0' || *line == '\n')
            break;

        (void)(
            ffParsePropLine(line, "model name :", &cpu->name) ||
            ffParsePropLine(line, "vendor_id :", &cpu->vendor) ||
            ffParsePropLine(line, "cpu cores :", physicalCoresBuffer) ||
            ffParsePropLine(line, "cpu MHz :", cpuMHz) ||
            ffParsePropLine(line, "isa :", cpuIsa) ||
            ffParsePropLine(line, "uarch :", cpuUarch) ||
            (cpu->name.length == 0 && ffParsePropLine(line, "Hardware :", &cpu->name)) || //For Android devices
            (cpu->name.length == 0 && ffParsePropLine(line, "cpu     :", &cpu->name)) || //For POWER
            (cpu->name.length == 0 && ffParsePropLine(line, "cpu model               :", &cpu->name)) //For MIPS
        );
    }

    return NULL;
}

static double getGHz(const char* file)
{
    FF_STRBUF_AUTO_DESTROY content = ffStrbufCreate();
    if(ffAppendFileBuffer(file, &content))
    {
        double herz = ffStrbufToDouble(&content);

        //ffStrbufToDouble failed
        if(herz != herz)
            return 0;

        herz /= 1000.0; //to MHz
        return herz / 1000.0; //to GHz
    }
    return 0;
}

static double getFrequency(const char* info, const char* scaling)
{
    double frequency = getGHz(info);
    if(frequency > 0.0)
        return frequency;

    return getGHz(scaling);
}

static double detectCPUTemp(void)
{
    const FFlist* tempsResult = ffDetectTemps();

    FF_LIST_FOR_EACH(FFTempValue, value, *tempsResult)
    {
        if(
            ffStrbufFirstIndexS(&value->name, "cpu") < value->name.length ||
            ffStrbufCompS(&value->name, "k10temp") == 0 ||
            ffStrbufCompS(&value->name, "coretemp") == 0
        ) return value->value;
    }

    return FF_CPU_TEMP_UNSET;
}

static void parseIsa(FFstrbuf* cpuIsa)
{
    if(ffStrbufStartsWithS(cpuIsa, "rv"))
    {
        // RISC-V ISA string example: "rv64imafdch_zicsr_zifencei".
        // The _z parts are not important for CPU showcasing, so we remove them.
        if(ffStrbufContainC(cpuIsa, '_'))
            ffStrbufSubstrBeforeFirstC(cpuIsa, '_');
        // Then we replace "imafd" with "g" since "g" is a shorthand.
        if(ffStrbufContainS(cpuIsa, "imafd"))
        {
            // Remove 4 of the 5 characters and replace the remaining one with "g".
            ffStrbufRemoveSubstr(cpuIsa, 4, 8);
            cpuIsa->chars[4] = 'g';
        }
        // The final ISA output of the above example is "rv64gch".
    }
    if(ffStrbufStartsWithS(cpuIsa, "mips"))
    {
        ffStrbufSubstrAfterLastC(cpuIsa, ' ');
    }
}

const char* ffDetectCPUImpl(const FFCPUOptions* options, FFCPUResult* cpu)
{
    cpu->temperature = options->temp ? detectCPUTemp() : FF_CPU_TEMP_UNSET;

    FF_STRBUF_AUTO_DESTROY physicalCoresBuffer = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY cpuMHz = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY cpuIsa = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY cpuUarch = ffStrbufCreate();

    const char* error = parseCpuInfo(cpu, &physicalCoresBuffer, &cpuMHz, &cpuIsa, &cpuUarch);
    if (error) return error;

    cpu->coresPhysical = (uint16_t) ffStrbufToUInt(&physicalCoresBuffer, 1);

    cpu->coresLogical = (uint16_t) get_nprocs_conf();
    cpu->coresOnline = (uint16_t) get_nprocs();

    #define BP "/sys/devices/system/cpu/cpufreq/policy0/"
    if(ffPathExists(BP, FF_PATHTYPE_DIRECTORY))
    {
        cpu->frequencyMin = getFrequency(BP"cpuinfo_min_freq", BP"scaling_min_freq");
        cpu->frequencyMax = getFrequency(BP"cpuinfo_max_freq", BP"scaling_max_freq");
    }
    else
    {
        cpu->frequencyMin = cpu->frequencyMax = ffStrbufToDouble(&cpuMHz) / 1000;
    }

    if(cpuUarch.length > 0)
    {
        if(cpu->name.length > 0)
            ffStrbufAppendC(&cpu->name, ' ');
        ffStrbufAppend(&cpu->name, &cpuUarch);
    }

    if(cpuIsa.length > 0)
    {
        parseIsa(&cpuIsa);
        if(cpu->name.length > 0)
            ffStrbufAppendC(&cpu->name, ' ');
        ffStrbufAppend(&cpu->name, &cpuIsa);
    }

    #ifdef __ANDROID__
    detectAndroid(cpu);
    #endif

    #ifdef __linux__
    if (cpu->name.length == 0)
    {
        FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
        if (ffProcessAppendStdOut(&buffer, (char *const[]) { "lscpu", NULL }) == NULL)
        {
            char* pstart = buffer.chars;
            while ((pstart = strstr(pstart, "Model name:")))
            {
                pstart += strlen("Model name:");
                while (isspace(*pstart)) ++pstart;
                if (*pstart == '\0')
                    break;

                if (cpu->name.length > 0)
                    ffStrbufAppendS(&cpu->name, " + ");

                char* pend = strchr(pstart, '\n');
                if (pend != NULL)
                    ffStrbufAppendNS(&cpu->name, (uint32_t) (pend - pstart), pstart);
                else
                {
                    ffStrbufAppendS(&cpu->name, pstart);
                    break;
                }

                pstart = pend + 1;
            }

            if (cpu->vendor.length == 0)
            {
                pstart = strstr(buffer.chars, "Vendor ID:");
                if (pstart)
                {
                    pstart += strlen("Vendor ID:");
                    while (isspace(*pstart)) ++pstart;
                    if (*pstart)
                    {
                        char* pend = strchr(pstart, '\n');
                        if (pend != NULL)
                            ffStrbufAppendNS(&cpu->vendor, (uint32_t) (pend - pstart), pstart);
                        else
                        {
                            ffStrbufAppendS(&cpu->vendor, pstart);
                        }
                    }
                }
            }
        }
    }
    #endif

    return NULL;
}

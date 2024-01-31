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

    #ifdef __aarch64__
    FF_STRBUF_AUTO_DESTROY implementer = ffStrbufCreate();
    #endif

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

            #ifdef __aarch64__
            (cpu->vendor.length == 0 && ffParsePropLine(line, "CPU implementer :", &implementer)) ||
            #endif

            (cpu->name.length == 0 && ffParsePropLine(line, "Hardware :", &cpu->name)) || //For Android devices
            (cpu->name.length == 0 && ffParsePropLine(line, "cpu     :", &cpu->name)) || //For POWER
            (cpu->name.length == 0 && ffParsePropLine(line, "cpu model               :", &cpu->name)) //For MIPS
        );
    }

    #ifdef __aarch64__
    // https://github.com/util-linux/util-linux/blob/2cd89de14549d2b2c079a4f8b73f75500d229fee/sys-utils/lscpu-arm.c#L286
    if (cpu->vendor.length == 0 && implementer.length > 2 /* 0xX */)
    {
        uint32_t implId = (uint32_t) strtoul(implementer.chars, NULL, 16);
        switch (implId)
        {
        case 0x41: ffStrbufSetStatic(&cpu->vendor, "ARM"); break;
        case 0x42: ffStrbufSetStatic(&cpu->vendor, "Broadcom"); break;
        case 0x48: ffStrbufSetStatic(&cpu->vendor, "HiSilicon"); break;
        case 0x4e: ffStrbufSetStatic(&cpu->vendor, "Nvidia"); break;
        case 0x51: ffStrbufSetStatic(&cpu->vendor, "Qualcomm"); break;
        case 0x53: ffStrbufSetStatic(&cpu->vendor, "Samsung"); break;
        case 0x61: ffStrbufSetStatic(&cpu->vendor, "Apple"); break;
        case 0x69: ffStrbufSetStatic(&cpu->vendor, "Intel"); break;
        }
    }
    #endif

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

void detectAsahi(FFCPUResult* cpu)
{
    // In Asahi Linux, reading /proc/device-tree/compatible gives
    // information on the device model. It consists of 3 NUL terminated
    // strings, the second of which gives the actual SoC model. But it
    // is not the marketing name, i.e. for M2 there is "apple,t8112" in
    // the compatible string.
    //
    // A full list of the SoC model names can be found here:
    // https://github.com/AsahiLinux/docs/wiki/Codenames
    if (cpu->name.length == 0 && ffStrbufEqualS(&cpu->vendor, "Apple"))
    {
        FF_STRBUF_AUTO_DESTROY content = ffStrbufCreate();
        char* cpu_compat = NULL;
        char* cpu_model_name = NULL;

        ffStrbufAppend(&cpu->name, &cpu->vendor);
        ffStrbufAppendC(&cpu->name, ' ');

        if (ffAppendFileBuffer("/proc/device-tree/compatible", &content))
        {
            // get the second NUL terminated string
            cpu_compat = content.chars + strlen(content.chars) + 1;
            // "apple,t8112" -> "t8112"
            cpu_compat = strchr(cpu_compat, ',') + 1;
            if (strcmp(cpu_compat, "t8103") == 0)
            {
                cpu_model_name = "M1";
            }
            else if (strcmp(cpu_compat, "t6000") == 0)
            {
                cpu_model_name = "M1 Pro";
            }
            else if (strcmp(cpu_compat, "t6001") == 0)
            {
                cpu_model_name = "M1 Max";
            }
            else if (strcmp(cpu_compat, "t6002") == 0)
            {
                cpu_model_name = "M1 Ultra";
            }
            else if (strcmp(cpu_compat, "t8112") == 0)
            {
                cpu_model_name = "M2";
            }
            else if (strcmp(cpu_compat, "t6020") == 0)
            {
                cpu_model_name = "M2 Pro";
            }
            else if (strcmp(cpu_compat, "t6021") == 0)
            {
                cpu_model_name = "M2 Max";
            }
            else if (strcmp(cpu_compat, "t6022") == 0)
            {
                cpu_model_name = "M2 Ultra";
            }
            else if (strcmp(cpu_compat, "t8122") == 0)
            {
                cpu_model_name = "M3";
            }
            else if (strcmp(cpu_compat, "t6030") == 0)
            {
                cpu_model_name = "M3 Pro";
            }
            else if (strcmp(cpu_compat, "t6031") == 0
                            || strcmp(cpu_compat, "t6034") == 0)
            {
                cpu_model_name = "M3 Max";
            }
            else
            {
                cpu_model_name = "CPU";
            }
        }

        ffStrbufAppendS(&cpu->name, cpu_model_name);
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

    cpu->coresLogical = (uint16_t) get_nprocs_conf();
    cpu->coresOnline = (uint16_t) get_nprocs();
    cpu->coresPhysical = (uint16_t) ffStrbufToUInt(&physicalCoresBuffer, cpu->coresLogical);

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

    #if defined(__linux__) && defined(__aarch64__)
    detectAsahi(cpu);
    #endif

    if (cpu->name.length == 0)
    {
        FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
        if (ffProcessAppendStdOut(&buffer, (char *const[]) { "lscpu", NULL }) == NULL)
        {
            char* pstart = buffer.chars;

            if (cpu->vendor.length == 0)
            {
                pstart = strstr(pstart, "Vendor ID:");
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
                        pstart = pend + 1;
                        if (pstart >= buffer.chars + buffer.length)
                            return NULL;
                    }
                }
            }

            while ((pstart = strstr(pstart, "Model name:")))
            {
                pstart += strlen("Model name:");
                while (isspace(*pstart)) ++pstart;
                if (*pstart == '\0')
                    break;

                if (cpu->name.length > 0)
                    ffStrbufAppendS(&cpu->name, " + ");

                if (*pstart == '-')
                {
                    if (cpu->vendor.length > 0)
                        ffStrbufAppend(&cpu->name, &cpu->vendor);
                    else
                        ffStrbufAppendS(&cpu->name, "Unknown");
                    ++pstart;
                    continue;
                }

                char* pend = strchr(pstart, '\n');
                if (pend != NULL)
                    ffStrbufAppendNS(&cpu->name, (uint32_t) (pend - pstart), pstart);
                else
                {
                    ffStrbufAppendS(&cpu->name, pstart);
                    break;
                }

                pstart = pend + 1;
                if (pstart >= buffer.chars + buffer.length)
                    return NULL;
            }
        }
    }

    return NULL;
}

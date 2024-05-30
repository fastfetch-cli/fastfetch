#include "cpu.h"
#include "common/io/io.h"
#include "common/processing.h"
#include "common/properties.h"
#include "detection/temps/temps_linux.h"
#include "util/mallocHelper.h"
#include "util/stringUtils.h"

#include <sys/sysinfo.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef __ANDROID__
#include "common/settings.h"

static void detectAndroid(FFCPUResult* cpu)
{
    if (cpu->name.length == 0)
    {
        ffSettingsGetAndroidProperty("ro.soc.model", &cpu->name);
        ffStrbufClear(&cpu->vendor); // We usually detect the vendor of CPU core as ARM, but instead we want the vendor of SOC
    }
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

static uint32_t getFrequency(FFstrbuf* basePath, const char* cpuinfoFileName, const char* scalingFileName, FFstrbuf* buffer)
{
    uint32_t baseLen = basePath->length;
    ffStrbufAppendS(basePath, cpuinfoFileName);
    bool ok = ffReadFileBuffer(basePath->chars, buffer);
    ffStrbufSubstrBefore(basePath, baseLen);
    if (ok)
        return (uint32_t) ffStrbufToUInt(buffer, 0);

    if (scalingFileName)
    {
        ffStrbufAppendS(basePath, scalingFileName);
        ok = ffReadFileBuffer(basePath->chars, buffer);
        ffStrbufSubstrBefore(basePath, baseLen);
        if (ok)
            return (uint32_t) ffStrbufToUInt(buffer, 0);
    }

    return 0;
}

static uint8_t getNumCores(FFstrbuf* basePath, FFstrbuf* buffer)
{
    uint32_t baseLen = basePath->length;
    ffStrbufAppendS(basePath, "/affected_cpus");
    bool ok = ffReadFileBuffer(basePath->chars, buffer);
    ffStrbufSubstrBefore(basePath, baseLen);
    if (ok)
        return (uint8_t) ffStrbufCountC(buffer, ' ') + 1;

    ffStrbufAppendS(basePath, "/related_cpus");
    ok = ffReadFileBuffer(basePath->chars, buffer);
    ffStrbufSubstrBefore(basePath, baseLen);
    if (ok)
        return (uint8_t) ffStrbufCountC(buffer, ' ') + 1;

    return 0;
}

static bool detectFrequency(FFCPUResult* cpu, const FFCPUOptions* options)
{
    FF_STRBUF_AUTO_DESTROY path = ffStrbufCreateS("/sys/devices/system/cpu/cpufreq/");
    FF_AUTO_CLOSE_DIR DIR* dir = opendir(path.chars);
    if (!dir) return false;

    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
    uint32_t baseLen = path.length;

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (ffStrStartsWith(entry->d_name, "policy") && ffCharIsDigit(entry->d_name[strlen("policy")]))
        {
            ffStrbufAppendS(&path, entry->d_name);
            uint32_t fbase = getFrequency(&path, "/base_frequency", NULL, &buffer);
            if (fbase > 0)
            {
                if (cpu->frequencyBase == cpu->frequencyBase)
                    cpu->frequencyBase = cpu->frequencyBase > fbase ? cpu->frequencyBase : fbase;
                else
                    cpu->frequencyBase = fbase;
            }
            uint32_t fbioslimit = getFrequency(&path, "/bios_limit", NULL, &buffer);
            if (fbioslimit > 0)
            {
                if (cpu->frequencyBiosLimit == cpu->frequencyBiosLimit)
                    cpu->frequencyBiosLimit = cpu->frequencyBiosLimit > fbioslimit ? cpu->frequencyBiosLimit : fbioslimit;
                else
                    cpu->frequencyBiosLimit = fbioslimit;
            }
            uint32_t fmax = getFrequency(&path, "/cpuinfo_max_freq", "/scaling_max_freq", &buffer);
            if (fmax > 0)
            {
                if (cpu->frequencyMax == cpu->frequencyMax)
                    cpu->frequencyMax = cpu->frequencyMax > fmax ? cpu->frequencyMax : fmax;
                else
                    cpu->frequencyMax = fmax;
            }
            uint32_t fmin = getFrequency(&path, "/cpuinfo_min_freq", "/scaling_min_freq", &buffer);
            if (fmin > 0)
            {
                if (cpu->frequencyMin == cpu->frequencyMin)
                    cpu->frequencyMin = cpu->frequencyMin < fmin ? cpu->frequencyMin : fmin;
                else
                    cpu->frequencyMin = fmin;
            }

            if (options->showPeCoreCount)
            {
                uint32_t freq = fbase == 0 ? fmax : fbase; // seems base frequencies are more stable
                uint32_t ifreq = 0;
                while (cpu->coreTypes[ifreq].freq != freq && cpu->coreTypes[ifreq].freq > 0)
                    ++ifreq;
                if (cpu->coreTypes[ifreq].freq == 0)
                    cpu->coreTypes[ifreq].freq = freq;
                cpu->coreTypes[ifreq].count += getNumCores(&path, &buffer);
            }
            ffStrbufSubstrBefore(&path, baseLen);
        }
    }
    cpu->frequencyBase /= 1e6;
    cpu->frequencyMax /= 1e6;
    cpu->frequencyMin /= 1e6;
    cpu->frequencyBiosLimit /= 1e6;
    return true;
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
    if (cpu->name.length == 0 && ffStrbufEqualS(&cpu->vendor, "Apple"))
    {
        char content[32];
        ssize_t length = ffReadFileData("/proc/device-tree/compatible", sizeof(content), content);
        if (length <= 0) return;

        // get the second NUL terminated string
        char* modelName = memchr(content, '\0', (size_t) length) + 1;
        if (modelName - content < length && ffStrStartsWith(modelName, "apple,t"))
        {
            uint32_t deviceId = (uint32_t) strtoul(modelName + strlen("apple,t"), NULL, 10);
            ffStrbufSetStatic(&cpu->name, ffCPUAppleCodeToName(deviceId));
        }
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

    if (!detectFrequency(cpu, options) || cpu->frequencyBase != cpu->frequencyBase)
        cpu->frequencyBase = ffStrbufToDouble(&cpuMHz) / 1000;

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
                else
                {
                    pstart = buffer.chars;
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

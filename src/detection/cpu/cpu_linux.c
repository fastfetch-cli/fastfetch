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

static void detectQualcomm(FFCPUResult* cpu)
{
    if (ffStrbufEqualS(&cpu->name, "SM8635"))
        ffStrbufSetStatic(&cpu->name, "Qualcomm Snapdragon 8s Gen 3 [SM8635]");
    else if (ffStrbufEqualS(&cpu->name, "SM8650-AC"))
        ffStrbufSetStatic(&cpu->name, "Qualcomm Snapdragon 8 Gen 3 for Galaxy [SM8650-AC]");
    else if (ffStrbufEqualS(&cpu->name, "SM8650"))
        ffStrbufSetStatic(&cpu->name, "Qualcomm Snapdragon 8 Gen 3 [SM8650]");
    else if (ffStrbufEqualS(&cpu->name, "SM8550-AC"))
        ffStrbufSetStatic(&cpu->name, "Qualcomm Snapdragon 8 Gen 2 for Galaxy [SM8550-AC]");
    else if (ffStrbufEqualS(&cpu->name, "SM8550"))
        ffStrbufSetStatic(&cpu->name, "Qualcomm Snapdragon 8 Gen 2 [SM8550]");
    else if (ffStrbufEqualS(&cpu->name, "SM8475"))
        ffStrbufSetStatic(&cpu->name, "Qualcomm Snapdragon 8+ Gen 1 [SM8475]");
    else if (ffStrbufEqualS(&cpu->name, "SM8450"))
        ffStrbufSetStatic(&cpu->name, "Qualcomm Snapdragon 8 Gen 1 [SM8450]");

    else if (ffStrbufEqualS(&cpu->name, "SM7675"))
        ffStrbufSetStatic(&cpu->name, "Qualcomm Snapdragon 7+ Gen 3 [SM7675]");
    else if (ffStrbufEqualS(&cpu->name, "SM7550"))
        ffStrbufSetStatic(&cpu->name, "Qualcomm Snapdragon 7 Gen 3 [SM7550]");
    else if (ffStrbufEqualS(&cpu->name, "SM7475"))
        ffStrbufSetStatic(&cpu->name, "Qualcomm Snapdragon 7+ Gen 2 [SM7550]");
    else if (ffStrbufEqualS(&cpu->name, "SM7435"))
        ffStrbufSetStatic(&cpu->name, "Qualcomm Snapdragon 7s Gen 2 [SM7435]");
    else if (ffStrbufEqualS(&cpu->name, "SM7450"))
        ffStrbufSetStatic(&cpu->name, "Qualcomm Snapdragon 7 Gen 1 [SM7450]");

    else if (ffStrbufEqualS(&cpu->name, "SM6375-AC"))
        ffStrbufSetStatic(&cpu->name, "Qualcomm Snapdragon 6s Gen 3 [SM6375-AC]");
    else if (ffStrbufEqualS(&cpu->name, "SM6450"))
        ffStrbufSetStatic(&cpu->name, "Qualcomm Snapdragon 6 Gen 1 [SM6450]");

    else if (ffStrbufEqualS(&cpu->name, "SM4635"))
        ffStrbufSetStatic(&cpu->name, "Qualcomm Snapdragon 4s Gen 2 [SM4635]");
    else if (ffStrbufEqualS(&cpu->name, "SM4450"))
        ffStrbufSetStatic(&cpu->name, "Qualcomm Snapdragon 4 Gen 2 [SM4450]");
    else if (ffStrbufEqualS(&cpu->name, "SM4375"))
        ffStrbufSetStatic(&cpu->name, "Qualcomm Snapdragon 4 Gen 1 [SM4375]");
}

static void detectAndroid(FFCPUResult* cpu)
{
    if (cpu->name.length == 0)
    {
        if (ffSettingsGetAndroidProperty("ro.soc.model", &cpu->name))
            ffStrbufClear(&cpu->vendor); // We usually detect the vendor of CPU core as ARM, but instead we want the vendor of SOC
        else if(ffSettingsGetAndroidProperty("ro.mediatek.platform", &cpu->name))
            ffStrbufSetStatic(&cpu->vendor, "MTK");
    }
    if (cpu->vendor.length == 0)
    {
        if (!ffSettingsGetAndroidProperty("ro.soc.manufacturer", &cpu->vendor))
            ffSettingsGetAndroidProperty("ro.product.product.manufacturer", &cpu->vendor);
    }

    if (ffStrbufEqualS(&cpu->vendor, "QTI") && ffStrbufStartsWithS(&cpu->name, "SM"))
        detectQualcomm(cpu);
}
#endif

#if __arm__ || __aarch64__
#include "cpu_arm.h"

static void detectArmName(FILE* cpuinfo, FFCPUResult* cpu, uint32_t implId)
{
    FF_AUTO_FREE char* line = NULL;
    rewind(cpuinfo);
    size_t len = 0;
    uint32_t lastPartId = UINT32_MAX;
    uint32_t num = 0;
    while(getline(&line, &len, cpuinfo) != -1)
    {
        if (!ffStrStartsWith(line, "CPU part\t: ")) continue;
        uint32_t partId = (uint32_t) strtoul(line + strlen("CPU part\t: "), NULL, 16);
        const char* name = NULL;
        if (partId > 0) // Linux reports 0 for unknown CPUs
        {
            switch (implId)
            {
                case 0x41: name = armPartId2name(partId); break;
                case 0x42: name = brcmPartId2name(partId); break;
                case 0x43: name = caviumPartId2name(partId); break;
                case 0x44: name = decPartId2name(partId); break;
                case 0x46: name = fujitsuPartId2name(partId); break;
                case 0x48: name = hisiPartId2name(partId); break;
                case 0x4e: name = nvidiaPartId2name(partId); break;
                case 0x50: name = apmPartId2name(partId); break;
                case 0x51: name = qcomPartId2name(partId); break;
                case 0x53: name = samsungPartId2name(partId); break;
                case 0x56: name = marvellPartId2name(partId); break;
                case 0x61: name = applePartId2name(partId); break;
                case 0x66: name = faradayPartId2name(partId); break;
                case 0x69: name = intelPartId2name(partId); break;
                case 0x6d: name = msPartId2name(partId); break;
                case 0x70: name = ftPartId2name(partId); break;
                case 0xc0: name = amperePartId2name(partId); break;
            }
        }
        if (lastPartId != partId)
        {
            if (lastPartId != UINT32_MAX)
            {
                if (num > 1)
                    ffStrbufAppendF(&cpu->name, "*%u", num);
                ffStrbufAppendS(&cpu->name, " + ");
            }
            if (name)
                ffStrbufAppendS(&cpu->name, name);
            else if (partId)
                ffStrbufAppendF(&cpu->name, "%s-%X", cpu->vendor.chars, partId);
            else
                ffStrbufAppend(&cpu->name, &cpu->vendor);
            lastPartId = partId;
            num = 1;
        }
        else
            ++num;
    }
    if (num > 1)
        ffStrbufAppendF(&cpu->name, "*%u", num);
}
#endif

static const char* parseCpuInfo(FILE* cpuinfo, FFCPUResult* cpu, FFstrbuf* physicalCoresBuffer, FFstrbuf* cpuMHz, FFstrbuf* cpuIsa, FFstrbuf* cpuUarch, FFstrbuf* cpuImplementer)
{
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

            #if __arm__ || __aarch64__
            (cpu->vendor.length == 0 && ffParsePropLine(line, "CPU implementer :", cpuImplementer)) ||
            #endif
            #if __ANDROID__
            (cpu->name.length == 0 && ffParsePropLine(line, "Hardware :", &cpu->name)) || //For Android devices
            #endif
            #if __powerpc__ || __powerpc
            (cpu->name.length == 0 && ffParsePropLine(line, "cpu     :", &cpu->name)) || //For POWER
            #endif
            #if __mips__
            (cpu->name.length == 0 && ffParsePropLine(line, "cpu model               :", &cpu->name)) || //For MIPS
            #endif
            false
        );
    }

    return NULL;
}

static uint32_t getFrequency(FFstrbuf* basePath, const char* cpuinfoFileName, const char* scalingFileName, FFstrbuf* buffer)
{
    uint32_t baseLen = basePath->length;
    ffStrbufAppendS(basePath, cpuinfoFileName);
    bool ok = ffReadFileBuffer(basePath->chars, buffer);
    ffStrbufSubstrBefore(basePath, baseLen);
    if (ok)
        return (uint32_t) (ffStrbufToUInt(buffer, 0) / 1000);

    if (scalingFileName)
    {
        ffStrbufAppendS(basePath, scalingFileName);
        ok = ffReadFileBuffer(basePath->chars, buffer);
        ffStrbufSubstrBefore(basePath, baseLen);
        if (ok)
            return (uint32_t) (ffStrbufToUInt(buffer, 0) / 1000);
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
        return (uint8_t) (ffStrbufCountC(buffer, ' ') + 1);

    ffStrbufAppendS(basePath, "/related_cpus");
    ok = ffReadFileBuffer(basePath->chars, buffer);
    ffStrbufSubstrBefore(basePath, baseLen);
    if (ok)
        return (uint8_t) (ffStrbufCountC(buffer, ' ') + 1);

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
                cpu->frequencyBase = cpu->frequencyBase > fbase ? cpu->frequencyBase : fbase;

            uint32_t fbioslimit = getFrequency(&path, "/bios_limit", NULL, &buffer);
            if (fbioslimit > 0)
                cpu->frequencyBiosLimit = cpu->frequencyBiosLimit > fbioslimit ? cpu->frequencyBiosLimit : fbioslimit;

            uint32_t fmax = getFrequency(&path, "/cpuinfo_max_freq", "/scaling_max_freq", &buffer);
            if (fmax > 0)
                cpu->frequencyMax = cpu->frequencyMax > fmax ? cpu->frequencyMax : fmax;

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
    FF_AUTO_CLOSE_FILE FILE* cpuinfo = fopen("/proc/cpuinfo", "r");
    if(cpuinfo == NULL)
        return "fopen(\"/proc/cpuinfo\", \"r\") failed";

    cpu->temperature = options->temp ? detectCPUTemp() : FF_CPU_TEMP_UNSET;

    FF_STRBUF_AUTO_DESTROY physicalCoresBuffer = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY cpuMHz = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY cpuIsa = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY cpuUarch = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY cpuImplementerStr = ffStrbufCreate();

    const char* error = parseCpuInfo(cpuinfo, cpu, &physicalCoresBuffer, &cpuMHz, &cpuIsa, &cpuUarch, &cpuImplementerStr);
    if (error) return error;

    cpu->coresLogical = (uint16_t) get_nprocs_conf();
    cpu->coresOnline = (uint16_t) get_nprocs();
    cpu->coresPhysical = (uint16_t) ffStrbufToUInt(&physicalCoresBuffer, cpu->coresLogical);

    if (!detectFrequency(cpu, options) || cpu->frequencyBase == 0)
        cpu->frequencyBase = (uint32_t) ffStrbufToUInt(&cpuMHz, 0);

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

    #if __arm__ || __aarch64__
    uint32_t cpuImplementer = (uint32_t) strtoul(cpuImplementerStr.chars, NULL, 16);
    ffStrbufSetStatic(&cpu->vendor, hwImplId2Vendor(cpuImplementer));

    #if __ANDROID__
    detectAndroid(cpu);
    #elif __aarch64__
    detectAsahi(cpu);
    #endif

    if (cpu->name.length == 0)
        detectArmName(cpuinfo, cpu, cpuImplementer);
    #endif

    return NULL;
}

#include "cpu.h"
#include "common/io/io.h"
#include "common/processing.h"
#include "common/properties.h"
#include "util/mallocHelper.h"
#include "util/stringUtils.h"

#include <sys/sysinfo.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>

static double parseHwmonDir(FFstrbuf* dir, FFstrbuf* buffer)
{
    //https://www.kernel.org/doc/Documentation/hwmon/sysfs-interface
    uint32_t dirLength = dir->length;
    ffStrbufAppendS(dir, "temp1_input");

    if(!ffReadFileBuffer(dir->chars, buffer))
    {
        // Some badly implemented system put temp file in /hwmonN/device
        ffStrbufSubstrBefore(dir, dirLength);
        ffStrbufAppendS(dir, "device/");
        dirLength = dir->length;
        ffStrbufAppendS(dir, "temp1_input");

        if(!ffReadFileBuffer(dir->chars, buffer))
            return 0.0/0.0;
    }

    ffStrbufSubstrBefore(dir, dirLength);

    double value = ffStrbufToDouble(buffer);// millidegree Celsius

    if(value != value)
        return 0.0/0.0;

    ffStrbufAppendS(dir, "name");
    if (!ffReadFileBuffer(dir->chars, buffer))
        return 0.0/0.0;

    ffStrbufTrimRightSpace(buffer);

    if(
        ffStrbufContainS(buffer, "cpu") ||
        ffStrbufEqualS(buffer, "k10temp") || // AMD
        ffStrbufEqualS(buffer, "coretemp") // Intel
    ) return value / 1000.;

    return 0.0/0.0;
}

static double detectCPUTemp(void)
{
    FF_STRBUF_AUTO_DESTROY baseDir = ffStrbufCreateA(64);
    ffStrbufAppendS(&baseDir, "/sys/class/hwmon/");

    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();

    uint32_t baseDirLength = baseDir.length;

    FF_AUTO_CLOSE_DIR DIR* dirp = opendir(baseDir.chars);
    if(dirp == NULL)
        return 0.0/0.0;

    struct dirent* entry;
    while((entry = readdir(dirp)) != NULL)
    {
        if(entry->d_name[0] == '.')
            continue;

        ffStrbufAppendS(&baseDir, entry->d_name);
        ffStrbufAppendC(&baseDir, '/');

        double result = parseHwmonDir(&baseDir, &buffer);
        if (result == result)
            return result;

        ffStrbufSubstrBefore(&baseDir, baseDirLength);
    }

    return 0.0/0.0;
}

#ifdef __ANDROID__
#include "common/settings.h"

static void detectQualcomm(FFCPUResult* cpu)
{
    // https://en.wikipedia.org/wiki/List_of_Qualcomm_Snapdragon_systems_on_chips

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
    else if (ffStrbufEqualS(&cpu->name, "SM7635"))
        ffStrbufSetStatic(&cpu->name, "Qualcomm Snapdragon 7s Gen 3 [SM7635]");
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
    else if (ffStrbufEqualS(&cpu->name, "SM6475"))
        ffStrbufSetStatic(&cpu->name, "Qualcomm Snapdragon 6 Gen 3 [SM6475]");
    else if (ffStrbufEqualS(&cpu->name, "SM6115"))
        ffStrbufSetStatic(&cpu->name, "Qualcomm Snapdragon 6s Gen 1 [SM6115]");
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

static const char* parseCpuInfo(
    FF_MAYBE_UNUSED FILE* cpuinfo,
    FF_MAYBE_UNUSED FFCPUResult* cpu,
    FF_MAYBE_UNUSED FFstrbuf* physicalCoresBuffer,
    FF_MAYBE_UNUSED FFstrbuf* cpuMHz,
    FF_MAYBE_UNUSED FFstrbuf* cpuIsa,
    FF_MAYBE_UNUSED FFstrbuf* cpuUarch,
    FF_MAYBE_UNUSED FFstrbuf* cpuImplementer)
{
    FF_AUTO_FREE char* line = NULL;
    size_t len = 0;

    while(getline(&line, &len, cpuinfo) != -1)
    {
        //Stop after reasonable information is acquired
        if((*line == '\0' || *line == '\n')
            #if __arm__ || __loongarch__
            && cpu->name.length > 0 // #1202 #1204
            #endif
        )
            break;

        (void)(
            // arm64 doesn't have "model name"; arm32 does have "model name" but its value is not useful.
            // "Hardware" should always be used in this case
            #if !(__arm__ || __aarch64__)
            (cpu->name.length == 0 && ffParsePropLine(line, "model name :", &cpu->name)) ||
            (cpu->vendor.length == 0 && ffParsePropLine(line, "vendor_id :", &cpu->vendor)) ||
            (physicalCoresBuffer->length == 0 && ffParsePropLine(line, "cpu cores :", physicalCoresBuffer)) ||
            (cpuMHz->length == 0 && ffParsePropLine(line, "cpu MHz :", cpuMHz)) ||
            #endif

            #if !(__x86_64__ || __i386__ || __arm__ || __aarch64__)
            (cpuIsa->length == 0 && ffParsePropLine(line, "isa :", cpuIsa)) ||
            (cpuUarch->length == 0 && ffParsePropLine(line, "uarch :", cpuUarch)) ||
            #endif

            #if __arm__ || __aarch64__
            (cpuImplementer->length == 0 && ffParsePropLine(line, "CPU implementer :", cpuImplementer)) ||
            (cpu->name.length == 0 && ffParsePropLine(line, "Hardware :", &cpu->name)) || //For Android devices
            #endif
            #if __powerpc__ || __powerpc
            (cpu->name.length == 0 && ffParsePropLine(line, "cpu :", &cpu->name)) || //For POWER
            #endif
            #if __mips__
            (cpu->name.length == 0 && ffParsePropLine(line, "cpu model :", &cpu->name)) || //For MIPS
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

            uint32_t fmax = getFrequency(&path, "/cpuinfo_max_freq", "/scaling_max_freq", &buffer);
            if (fmax == 0) continue;

            if (cpu->frequencyMax >= fmax)
            {
                if (!options->showPeCoreCount)
                {
                    ffStrbufSubstrBefore(&path, baseLen);
                    continue;
                }
            }
            else
                cpu->frequencyMax = fmax;

            uint32_t fbase = getFrequency(&path, "/base_frequency", NULL, &buffer);
            if (fbase > 0)
                cpu->frequencyBase = cpu->frequencyBase > fbase ? cpu->frequencyBase : fbase;

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

FF_MAYBE_UNUSED static void parseIsa(FFstrbuf* cpuIsa)
{
    // Always use the last part of the ISA string. Ref: #590 #1204
    ffStrbufSubstrAfterLastC(cpuIsa, ' ');

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
}

FF_MAYBE_UNUSED static void detectArmSoc(FFCPUResult* cpu)
{
    if (cpu->name.length > 0)
        return;

    // device-vendor,device-model\0soc-vendor,soc-model\0
    char content[256];
    ssize_t length = ffReadFileData("/proc/device-tree/compatible", ARRAY_SIZE(content), content);
    if (length <= 2) return;

    // get the second NUL terminated string if it exists
    char* vendor = memchr(content, '\0', (size_t) length) + 1;
    if (!vendor || vendor - content >= length) vendor = content;

    char* model = strchr(vendor, ',');
    if (!model) return;
    *model = '\0';
    ++model;

    if (false) {}
    #if __aarch64__
    else if (ffStrEquals(vendor, "apple"))
    {
        // https://elixir.bootlin.com/linux/v6.11/source/arch/arm64/boot/dts/apple
        if (model[0] == 't')
        {
            uint32_t deviceId = (uint32_t) strtoul(model + 1, NULL, 10);
            ffStrbufSetStatic(&cpu->name, ffCPUAppleCodeToName(deviceId));

            if (!cpu->name.length)
            {
                ffStrbufSetS(&cpu->name, "Apple Silicon ");
                ffStrbufAppendS(&cpu->name, model);
            }
        }
        else
            ffStrbufSetS(&cpu->name, model);

        ffStrbufSetStatic(&cpu->vendor, "Apple");
    }
    #endif
    else if (ffStrEquals(vendor, "qcom"))
    {
        // https://elixir.bootlin.com/linux/v6.11/source/arch/arm64/boot/dts/qcom
        if (ffStrStartsWith(model, "x"))
        {
            ffStrbufSetS(&cpu->name, "Qualcomm Snapdragon X Elite ");
            for (const char* p = model + 1; *p; ++p)
                ffStrbufAppendC(&cpu->name, (char) toupper(*p));
        }
        else if (ffStrStartsWith(model, "sc"))
        {
            const char* code = model + 2;
            uint32_t deviceId = (uint32_t) strtoul(code, NULL, 10);
            ffStrbufSetStatic(&cpu->name, ffCPUQualcommCodeToName(deviceId));
            if (!cpu->name.length)
            {
                ffStrbufAppendS(&cpu->name, "Qualcomm Snapdragon SC");
                ffStrbufAppendS(&cpu->name, code);
            }
        }
        else
            ffStrbufSetS(&cpu->name, model);

        ffStrbufSetStatic(&cpu->vendor, "Qualcomm");
    }
    else if (ffStrEquals(vendor, "brcm"))
    {
        // Raspberry Pi
        ffStrbufSetStatic(&cpu->vendor, "Broadcom");
        for (const char* p = model; *p; ++p)
            ffStrbufAppendC(&cpu->name, (char) toupper(*p));
    }
    else
    {
        ffStrbufSetS(&cpu->name, model);
        ffStrbufSetS(&cpu->vendor, vendor);
        cpu->vendor.chars[0] = (char) toupper(vendor[0]);
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

    // Ref https://github.com/fastfetch-cli/fastfetch/issues/1194#issuecomment-2295058252
    ffCPUDetectSpeedByCpuid(cpu);
    if (!detectFrequency(cpu, options) || cpu->frequencyBase == 0)
        cpu->frequencyBase = (uint32_t) ffStrbufToUInt(&cpuMHz, 0);

    #if !(__x86_64__ || __i386__ || __arm__ || __aarch64__)
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
    #endif

    #if __arm__ || __aarch64__
    uint32_t cpuImplementer = (uint32_t) strtoul(cpuImplementerStr.chars, NULL, 16);
    ffStrbufSetStatic(&cpu->vendor, hwImplId2Vendor(cpuImplementer));

    #if __ANDROID__
    detectAndroid(cpu);
    #else
    detectArmSoc(cpu);
    #endif

    if (cpu->name.length == 0)
        detectArmName(cpuinfo, cpu, cpuImplementer);
    #endif

    return NULL;
}

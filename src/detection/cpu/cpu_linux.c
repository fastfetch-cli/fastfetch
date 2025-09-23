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
#include <fcntl.h>

#define FF_CPUINFO_PATH "/proc/cpuinfo"

static double parseTZDir(int dfd, FFstrbuf* buffer)
{
    if (!ffReadFileBufferRelative(dfd, "type", buffer))
        return FF_CPU_TEMP_UNSET;

    if (!ffStrbufStartsWithS(buffer, "cpu") &&
        !ffStrbufStartsWithS(buffer, "soc") &&
        #if __x86_64__ || __i386__
        !ffStrbufEqualS(buffer, "x86_pkg_temp") &&
        #endif
        true
    ) return FF_CPU_TEMP_UNSET;

    if (!ffReadFileBufferRelative(dfd, "temp", buffer))
        return FF_CPU_TEMP_UNSET;

    double value = ffStrbufToDouble(buffer, FF_CPU_TEMP_UNSET);// millidegree Celsius
    if (value == FF_CPU_TEMP_UNSET)
        return FF_CPU_TEMP_UNSET;

    return value / 1000.;
}

static double parseHwmonDir(int dfd, FFstrbuf* buffer)
{
    if (!ffReadFileBufferRelative(dfd, "name", buffer))
        return FF_CPU_TEMP_UNSET;

    ffStrbufTrimRightSpace(buffer);

    if (
        !ffStrbufContainS(buffer, "cpu") &&
        #if __x86_64__ || __i386__
        !ffStrbufEqualS(buffer, "k10temp") && // AMD
        !ffStrbufEqualS(buffer, "fam15h_power") && // AMD
        !ffStrbufEqualS(buffer, "coretemp") && // Intel
        #endif
        true
    ) return FF_CPU_TEMP_UNSET;

    //https://www.kernel.org/doc/Documentation/hwmon/sysfs-interface
    if (!ffReadFileBufferRelative(dfd, "temp1_input", buffer))
        return FF_CPU_TEMP_UNSET;

    double value = ffStrbufToDouble(buffer, FF_CPU_TEMP_UNSET);// millidegree Celsius
    if (value == FF_CPU_TEMP_UNSET)
        return FF_CPU_TEMP_UNSET;

    return value / 1000.;
}

static double detectCPUTemp(void)
{
    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
    {
        FF_AUTO_CLOSE_DIR DIR* dirp = opendir("/sys/class/hwmon/");
        if(dirp)
        {
            int dfd = dirfd(dirp);
            struct dirent* entry;
            while((entry = readdir(dirp)) != NULL)
            {
                if(entry->d_name[0] == '.')
                    continue;

                FF_AUTO_CLOSE_FD int subfd = openat(dfd, entry->d_name, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
                if(subfd < 0)
                    continue;

                double result = parseHwmonDir(subfd, &buffer);
                if (result != FF_CPU_TEMP_UNSET)
                    return result;
            }
        }
    }
    {
        FF_AUTO_CLOSE_DIR DIR* dirp = opendir("/sys/class/thermal/");
        if(dirp)
        {
            int dfd = dirfd(dirp);
            struct dirent* entry;
            while((entry = readdir(dirp)) != NULL)
            {
                if(entry->d_name[0] == '.')
                    continue;
                if(!ffStrStartsWith(entry->d_name, "thermal_zone"))
                    continue;

                FF_AUTO_CLOSE_FD int subfd = openat(dfd, entry->d_name, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
                if(subfd < 0)
                    continue;

                double result = parseTZDir(subfd, &buffer);
                if (result != FF_CPU_TEMP_UNSET)
                    return result;
            }
        }
    }
    {
        FF_AUTO_CLOSE_DIR DIR* dirp = opendir("/sys/devices/platform/");
        if(dirp)
        {
            int dfd = dirfd(dirp);
            struct dirent* entry;
            while((entry = readdir(dirp)) != NULL)
            {
                if(entry->d_name[0] == '.')
                    continue;
                if(!ffStrStartsWith(entry->d_name, "cputemp."))
                    continue;

                FF_AUTO_CLOSE_FD int subfd = openat(dfd, entry->d_name, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
                if(subfd < 0)
                    continue;

                double result = parseHwmonDir(subfd, &buffer);
                if (result != FF_CPU_TEMP_UNSET)
                    return result;
            }
        }
    }

    return FF_CPU_TEMP_UNSET;
}

#ifdef __ANDROID__
#include "common/settings.h"

static void detectQualcomm(FFCPUResult* cpu)
{
    // https://en.wikipedia.org/wiki/List_of_Qualcomm_Snapdragon_systems_on_chips

    uint32_t code = (uint32_t) strtoul(cpu->name.chars + 2, NULL, 10);
    const char* name = NULL;

    switch (code)
    {
        case 8845: name = "8 Gen 5"; break; // ?
        case 8850: name = "8 Elite Gen 5"; break;
        case 8735: name = "8s Gen 4"; break;
        case 8750: name = "8 Elite"; break;
        case 8635: name = "8s Gen 3"; break;
        case 8650: name = "8 Gen 3"; break;
        case 8550: name = "8 Gen 2"; break;
        case 8475: name = "8+ Gen 1"; break;
        case 8450: name = "8 Gen 1"; break;
        case 7750: name = "7 Gen 4"; break;
        case 7675: name = "7+ Gen 3"; break;
        case 7635: name = "7s Gen 3"; break;
        case 7550: name = "7 Gen 3"; break;
        case 7475: name = "7+ Gen 2"; break;
        case 7435: name = "7s Gen 2"; break;
        case 7450: name = "7 Gen 1"; break;
        case 6650: name = "6 Gen 4"; break;
        case 6375: name = "6s Gen 3"; break;
        case 6475: name = "6 Gen 3"; break;
        case 6115: name = "6s Gen 1"; break;
        case 6450: name = "6 Gen 1"; break;
        case 4635: name = "4s Gen 2"; break;
        case 4450: name = "4 Gen 2"; break;
        case 4375: name = "4 Gen 1"; break;
    }

    if (name)
    {
        char str[32];
        ffStrCopy(str, cpu->name.chars, sizeof(str));
        ffStrbufSetF(&cpu->name, "Qualcomm Snapdragon %s [%s]", name, str);
        return;
    }
}

static void detectMediaTek(FFCPUResult* cpu)
{
    // https://en.wikipedia.org/wiki/List_of_MediaTek_systems_on_chips

    uint32_t code = (uint32_t) strtoul(cpu->name.chars + 2, NULL, 10);
    const char* name = NULL;

    switch (code) // The SOC code of MTK Dimensity series is full of mess
    {
        case 6993: name = "9500"; break;
        case 6991: name = "9400"; break;
        case 6989:
        case 8796: name = "9300"; break;
        case 6985: name = "9200"; break;
        case 6983:
        case 8798: name = "9000"; break;

        case 6899: name = "8400"; break;
        case 6897:
        case 8792: name = "8300"; break;
        case 6896: name = "8200"; break;
        case 8795: name = "8100"; break;
        case 6895: name = "8000"; break;
    }

    if (name)
    {
        char str[32];
        ffStrCopy(str, cpu->name.chars, sizeof(str));
        ffStrbufSetF(&cpu->name, "MediaTek Dimensity %s [%s]", name, str);
        return;
    }
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
    else if (ffStrbufEqualS(&cpu->vendor, "MTK") && ffStrbufStartsWithS(&cpu->name, "MT"))
        detectMediaTek(cpu);
}
#endif

#if __arm__ || __aarch64__
#include "cpu_arm.h"

static void detectArmName(FFstrbuf* cpuinfo, FFCPUResult* cpu, uint32_t implId)
{
    char* line = NULL;
    size_t len = 0;
    uint32_t lastPartId = UINT32_MAX;
    uint32_t num = 0;
    while(ffStrbufGetline(&line, &len, cpuinfo))
    {
        if (!ffStrStartsWith(line, "CPU part\t: ")) continue;
        uint32_t partId = (uint32_t) strtoul(line + strlen("CPU part\t: "), NULL, 16);
        const char* name = NULL;
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
            case 0x61:
                if (partId == 0)
                {
                    // https://github.com/Dr-Noob/cpufetch/issues/213#issuecomment-1927782105
                    ffStrbufSetStatic(&cpu->name, "Virtualized Apple Silicon");
                    ffStrbufGetlineRestore(&line, &len, cpuinfo);
                    return;
                }
                name = applePartId2name(partId);
                break;
            case 0x66: name = faradayPartId2name(partId); break;
            case 0x69: name = intelPartId2name(partId); break;
            case 0x6d: name = msPartId2name(partId); break;
            case 0x70: name = ftPartId2name(partId); break;
            case 0xc0: name = amperePartId2name(partId); break;
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
    FFstrbuf* cpuinfo,
    FFCPUResult* cpu,
    FF_MAYBE_UNUSED FFstrbuf* physicalCoresBuffer,
    FF_MAYBE_UNUSED FFstrbuf* cpuMHz,
    FF_MAYBE_UNUSED FFstrbuf* cpuIsa,
    FF_MAYBE_UNUSED FFstrbuf* cpuUarch,
    FF_MAYBE_UNUSED FFstrbuf* cpuImplementer)
{
    char* line = NULL;
    size_t len = 0;

    while(ffStrbufGetline(&line, &len, cpuinfo))
    {
        //Stop after reasonable information is acquired
        if((*line == '\0' || *line == '\n') && cpu->name.length > 0)
        {
            ffStrbufGetlineRestore(&line, &len, cpuinfo);
            break;
        }

        (void)(
            // arm64 doesn't have "model name"; arm32 does have "model name" but its value is not useful.
            // "Hardware" should always be used in this case
            #if __x86_64__ || __i386__
            (cpu->name.length == 0 && ffParsePropLine(line, "model name :", &cpu->name)) ||
            (cpu->vendor.length == 0 && ffParsePropLine(line, "vendor_id :", &cpu->vendor)) ||
            (physicalCoresBuffer->length == 0 && ffParsePropLine(line, "cpu cores :", physicalCoresBuffer)) ||
            (cpuMHz->length == 0 && ffParsePropLine(line, "cpu MHz :", cpuMHz)) ||
            #elif __arm__ || __aarch64__
            (cpuImplementer->length == 0 && ffParsePropLine(line, "CPU implementer :", cpuImplementer)) ||
            (cpu->name.length == 0 && ffParsePropLine(line, "Hardware :", &cpu->name)) || //For Android devices
            #elif __powerpc__ || __powerpc
            (cpuMHz->length == 0 && ffParsePropLine(line, "clock :", cpuMHz)) ||
            (cpu->name.length == 0 && ffParsePropLine(line, "cpu :", &cpu->name)) ||
            #elif __mips__ || __mips
            (cpu->name.length == 0 && ffParsePropLine(line, "cpu model :", &cpu->name)) ||
            #elif __loongarch__
            (cpu->name.length == 0 && ffParsePropLine(line, "Model Name :", &cpu->name)) ||
            (cpuMHz->length == 0 && ffParsePropLine(line, "CPU MHz :", cpuMHz)) ||
            #elif __riscv__ || __riscv
            (cpuIsa->length == 0 && ffParsePropLine(line, "isa :", cpuIsa)) ||
            (cpuUarch->length == 0 && ffParsePropLine(line, "uarch :", cpuUarch)) ||
            #elif __s390x__
            (cpu->name.length == 0 && ffParsePropLine(line, "processor 0:", &cpu->name)) ||
            (cpu->vendor.length == 0 && ffParsePropLine(line, "vendor_id :", &cpu->vendor)) ||
            (cpuMHz->length == 0 && ffParsePropLine(line, "cpu MHz static :", cpuMHz)) || // This one cannot be detected because of early return
            #elif __ia64__
            (cpu->name.length == 0 && ffParsePropLine(line, "model name :", &cpu->name)) ||
            (cpu->vendor.length == 0 && ffParsePropLine(line, "vendor :", &cpu->vendor)) ||
            (cpuMHz->length == 0 && ffParsePropLine(line, "cpu MHz :", cpuMHz)) ||
            #else
            (cpu->name.length == 0 && ffParsePropLine(line, "model name :", &cpu->name)) ||
            (cpu->name.length == 0 && ffParsePropLine(line, "model :", &cpu->name)) ||
            (cpu->name.length == 0 && ffParsePropLine(line, "cpu model :", &cpu->name)) ||
            (cpu->name.length == 0 && ffParsePropLine(line, "hardware :", &cpu->name)) ||
            (cpu->name.length == 0 && ffParsePropLine(line, "processor :", &cpu->name)) ||
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

#if __i386__ || __x86_64__

FF_MAYBE_UNUSED static uint16_t getPackageCount(FFstrbuf* cpuinfo)
{
    const char* p = cpuinfo->chars;
    uint64_t low = 0, high = 0;

    while ((p = memmem(p, cpuinfo->length - (uint32_t) (p - cpuinfo->chars), "\nphysical id\t:", strlen("\nphysical id\t:"))))
    {
        p += strlen("\nphysical id\t:");
        char* pend;
        unsigned long long id = strtoul(p, &pend, 10);
        if (__builtin_expect(id > 64, false)) // Do 129-socket boards exist?
            high |= 1ULL << (id - 64);
        else
            low |= 1ULL << id;
        p = pend;
    }

    return (uint16_t) (__builtin_popcountll(low) + __builtin_popcountll(high));
}

FF_MAYBE_UNUSED static const char* detectCPUX86(const FFCPUOptions* options, FFCPUResult* cpu)
{
    FF_STRBUF_AUTO_DESTROY cpuinfo = ffStrbufCreateA(PROC_FILE_BUFFSIZ);
    if (!ffReadFileBuffer(FF_CPUINFO_PATH, &cpuinfo) || cpuinfo.length == 0)
        return "ffReadFileBuffer(\"" FF_CPUINFO_PATH "\") failed";

    FF_STRBUF_AUTO_DESTROY physicalCoresBuffer = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY cpuMHz = ffStrbufCreate();
    const char* error = parseCpuInfo(&cpuinfo, cpu, &physicalCoresBuffer, &cpuMHz, NULL,NULL, NULL);
    if (error) return error;

    cpu->coresLogical = (uint16_t) get_nprocs_conf();
    cpu->coresOnline = (uint16_t) get_nprocs();
    cpu->packages = getPackageCount(&cpuinfo);
    cpu->coresPhysical = (uint16_t) ffStrbufToUInt(&physicalCoresBuffer, 0); // physical cores in single package
    if (cpu->coresPhysical > 0 && cpu->packages > 1)
        cpu->coresPhysical *= cpu->packages;

    // Ref https://github.com/fastfetch-cli/fastfetch/issues/1194#issuecomment-2295058252
    ffCPUDetectByCpuid(cpu);
    if (!detectFrequency(cpu, options) || cpu->frequencyBase == 0)
        cpu->frequencyBase = (uint32_t) ffStrbufToUInt(&cpuMHz, 0);

    return NULL;
}

#else

static const char* detectPhysicalCores(FFCPUResult* cpu)
{
    int dfd = open("/sys/devices/system/cpu/", O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (dfd < 0) return "open(\"/sys/devices/system/cpu/\") failed";

    FF_AUTO_CLOSE_DIR DIR* dir = fdopendir(dfd);
    if (!dir) return "fdopendir(dfd) failed";

    uint64_t pkgLow = 0, pkgHigh = 0;

    struct dirent* entry;
    FF_LIST_AUTO_DESTROY cpuList = ffListCreate(sizeof(uint32_t));
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type != DT_DIR || !ffStrStartsWith(entry->d_name, "cpu") || !ffCharIsDigit(entry->d_name[strlen("cpu")]))
            continue;

        FF_AUTO_CLOSE_FD int cpuxfd = openat(dirfd(dir), entry->d_name, O_RDONLY | O_DIRECTORY);
        if (cpuxfd < 0)
            continue;

        char buf[128];

        // Check if the directory contains a file named "topology/physical_package_id"
        // that lists the physical package id of the CPU.

        ssize_t len = ffReadFileDataRelative(cpuxfd, "topology/physical_package_id", sizeof(buf) - 1, buf);
        if (len > 0)
        {
            buf[len] = '\0';
            unsigned long long id = strtoul(buf, NULL, 10);
            if (__builtin_expect(id > 64, false)) // Do 129-socket boards exist?
                pkgHigh |= 1ULL << (id - 64);
            else
                pkgLow |= 1ULL << id;
        }

        // Check if the directory contains a file named "topology/core_cpus_list"
        // that lists the physical cores in the package.

        len = ffReadFileDataRelative(cpuxfd, "topology/core_cpus_list", sizeof(buf) - 1, buf);
        if (len > 0)
        {
            buf[len] = '\0'; // low-high or low

            for (const char* p = buf; *p;)
            {
                char* pend;
                uint32_t coreId = (uint32_t) strtoul(p, &pend, 10);
                if (pend == p) break;

                bool found = false;
                FF_LIST_FOR_EACH(uint32_t, id, cpuList)
                {
                    if (*id == coreId)
                    {
                        // This core is already counted
                        found = true;
                        break;
                    }
                }
                if (!found)
                    *(uint32_t*) ffListAdd(&cpuList) = coreId;

                p = strchr(pend, ',');
                if (!p) break;
                ++p;
            }
        }
    }

    cpu->coresPhysical = (uint16_t) cpuList.length;
    cpu->packages = (uint16_t) (__builtin_popcountll(pkgLow) + __builtin_popcountll(pkgHigh));
    return NULL;
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

FF_MAYBE_UNUSED static void detectSocName(FFCPUResult* cpu)
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

#ifdef __loongarch__
FF_MAYBE_UNUSED static uint16_t getLoongarchPropCount(FFstrbuf* cpuinfo, const char* key)
{
    const char* p = cpuinfo->chars;
    uint64_t low = 0, high = 0;
    uint32_t keylen = (uint32_t) strlen(key);

    while ((p = memmem(p, cpuinfo->length - (uint32_t) (p - cpuinfo->chars), key, keylen)))
    {
        p += keylen;
        char* pend;
        unsigned long id = strtoul(p, &pend, 10);
        if (__builtin_expect(id > 64, false))
            high |= 1 << (id - 64);
        else
            low |= 1 << id;
        p = pend;
    }

    return (uint16_t) (__builtin_popcountll(low) + __builtin_popcountll(high));
}
#endif

FF_MAYBE_UNUSED static const char* detectCPUOthers(const FFCPUOptions* options, FFCPUResult* cpu)
{
    cpu->coresLogical = (uint16_t) get_nprocs_conf();
    cpu->coresOnline = (uint16_t) get_nprocs();

    #if __ANDROID__
    detectAndroid(cpu);
    #elif !__powerpc__ && !__powerpc
    detectSocName(cpu);
    #endif

    detectFrequency(cpu, options);

    if (cpu->name.length == 0)
    {
        FF_STRBUF_AUTO_DESTROY cpuinfo = ffStrbufCreateA(PROC_FILE_BUFFSIZ);
        if (!ffReadFileBuffer(FF_CPUINFO_PATH, &cpuinfo) || cpuinfo.length == 0)
            return "ffReadFileBuffer(\"" FF_CPUINFO_PATH "\") failed";

        FF_STRBUF_AUTO_DESTROY cpuMHz = ffStrbufCreate();
        FF_STRBUF_AUTO_DESTROY cpuIsa = ffStrbufCreate();
        FF_STRBUF_AUTO_DESTROY cpuUarch = ffStrbufCreate();
        FF_STRBUF_AUTO_DESTROY cpuImplementerStr = ffStrbufCreate();

        const char* error = parseCpuInfo(&cpuinfo, cpu, NULL, &cpuMHz, &cpuIsa, &cpuUarch, &cpuImplementerStr);
        if (error) return error;

        if (cpu->frequencyBase == 0)
            cpu->frequencyBase = (uint32_t) ffStrbufToUInt(&cpuMHz, 0);

        #if __arm__ || __aarch64__
        uint32_t cpuImplementer = (uint32_t) strtoul(cpuImplementerStr.chars, NULL, 16);
        ffStrbufSetStatic(&cpu->vendor, hwImplId2Vendor(cpuImplementer));

        if (cpu->name.length == 0)
            detectArmName(&cpuinfo, cpu, cpuImplementer);
        #elif __riscv__ || __riscv
        if (cpu->name.length == 0)
        {
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
        }
        #elif __loongarch__
        cpu->packages = getLoongarchPropCount(&cpuinfo, "\npackage\t\t\t:");
        cpu->coresPhysical = getLoongarchPropCount(&cpuinfo, "\ncore\t\t\t:");
        if (cpu->packages > 1) cpu->coresPhysical *= cpu->packages;
        #elif __s390x__
        if (ffStrbufSubstrAfterFirstS(&cpu->name, "machine = "))
            ffStrbufPrependS(&cpu->name, "Machine ");
        #endif
    }

    if (cpu->coresPhysical == 0)
        detectPhysicalCores(cpu);

    ffCPUDetectByCpuid(cpu);

    return NULL;
}
#endif

const char* ffDetectCPUImpl(const FFCPUOptions* options, FFCPUResult* cpu)
{
    cpu->temperature = options->temp ? detectCPUTemp() : FF_CPU_TEMP_UNSET;

    #if __x86_64__ || __i386__
    return detectCPUX86(options, cpu);
    #else
    return detectCPUOthers(options, cpu);
    #endif
}

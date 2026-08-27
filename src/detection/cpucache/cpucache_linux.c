#include "cpucache.h"
#include "common/io.h"
#include "common/strutil.h"

static const char* parseCpuCacheIndex(FFstrbuf* path, FFCPUCacheResult* result, FFstrbuf* buffer, FFstrbuf* added) {
    uint32_t baseLen = path->length;
    ffStrbufAppendS(path, "/level");
    if (!ffReadFileBuffer(path->chars, buffer)) {
        return "ffReadFileBuffer(\"/sys/devices/system/cpu/cpuX/cache/indexX/level\") == nullptr";
    }

    uint32_t level = (uint32_t) ffStrbufToUInt(buffer, 0);
    if (level < 1 || level > 4) {
        return "level < 1 || level > 4";
    }

    ffStrbufSubstrBefore(path, baseLen);
    ffStrbufAppendS(path, "/size");
    if (!ffReadFileBuffer(path->chars, buffer)) {
        return "ffReadFileBuffer(\"/sys/devices/system/cpu/cpuX/cache/indexX/size\") == nullptr";
    }

    uint32_t sizeKb = (uint32_t) ffStrbufToUInt(buffer, 0);
    if (sizeKb == 0) {
        return "size == 0";
    }

    ffStrbufSubstrBefore(path, baseLen);
    ffStrbufAppendS(path, "/type");
    if (!ffReadFileBuffer(path->chars, buffer)) {
        return "ffReadFileBuffer(\"/sys/devices/system/cpu/cpuX/cache/indexX/type\") == nullptr";
    }
    ffStrbufTrimRightSpace(buffer);

    FFCPUCacheType cacheType = 0;
    switch (buffer->chars[0]) {
        case 'I':
            cacheType = FF_CPU_CACHE_TYPE_INSTRUCTION;
            break;
        case 'D':
            cacheType = FF_CPU_CACHE_TYPE_DATA;
            break;
        case 'U':
            cacheType = FF_CPU_CACHE_TYPE_UNIFIED;
            break;
        case 'T':
            cacheType = FF_CPU_CACHE_TYPE_TRACE;
            break;
        default:
            return "unknown cache type";
    }

    uint32_t lineSize = 0;
    ffStrbufSubstrBefore(path, baseLen);
    ffStrbufAppendS(path, "/coherency_line_size");
    if (ffReadFileBuffer(path->chars, buffer)) {
        lineSize = (uint32_t) ffStrbufToUInt(buffer, 0);
    }

    ffStrbufSubstrBefore(path, baseLen);
    ffStrbufAppendS(path, "/shared_cpu_list");
    ffStrbufClear(buffer);
    ffStrbufAppendC(buffer, '[');
    if (!ffAppendFileBuffer(path->chars, buffer)) {
        return "ffAppendFileBuffer(\"/sys/devices/system/cpu/cpuX/cache/indexX/shared_cpu_list\") == nullptr";
    }
    ffStrbufTrimRightSpace(buffer);

    // deduplicate shared caches
    ffStrbufAppendF(buffer, "_%u_%u_%u_%u]", level, sizeKb, lineSize, cacheType);

    if (ffStrbufContain(added, buffer)) {
        return nullptr;
    }
    ffStrbufAppend(added, buffer);
    ffCPUCacheAddItem(result, level, sizeKb * 1024, lineSize, cacheType);
    return nullptr;
}

static const char* parseCpuCache(FFstrbuf* path, FFCPUCacheResult* result, FFstrbuf* buffer, FFstrbuf* added) {
    ffStrbufAppendS(path, "/cache/");
    uint32_t baseLen = path->length;
    FF_AUTO_CLOSE_DIR DIR* pathCacheDir = opendir(path->chars);
    if (!pathCacheDir) {
        return "opendir(\"/sys/devices/system/cpu/cpuX/cache/\") == nullptr";
    }

    struct dirent* pathCacheEntry;
    while ((pathCacheEntry = readdir(pathCacheDir)) != nullptr) {
        if (!ffStrStartsWith(pathCacheEntry->d_name, "index") || !ffCharIsDigit(pathCacheEntry->d_name[strlen("index")])) {
            continue;
        }

        ffStrbufAppendS(path, pathCacheEntry->d_name);
        const char* error = parseCpuCacheIndex(path, result, buffer, added);
        if (error) {
            return error;
        }
        ffStrbufSubstrBefore(path, baseLen);
    }

    return nullptr;
}

const char* ffDetectCPUCache(FFCPUCacheResult* result) {
    // https://www.kernel.org/doc/Documentation/ABI/testing/sysfs-devices-system-cpu
    FF_STRBUF_AUTO_DESTROY path = ffStrbufCreateS("/sys/devices/system/cpu/");
    uint32_t baseLen = path.length;
    FF_AUTO_CLOSE_DIR DIR* pathCpuDir = opendir(path.chars);
    if (!pathCpuDir) {
        return "opendir(\"/sys/devices/system/cpu/\") == nullptr";
    }

    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY added = ffStrbufCreate();

    struct dirent* pathCpuEntry;
    while ((pathCpuEntry = readdir(pathCpuDir)) != nullptr) {
        if (!ffStrStartsWith(pathCpuEntry->d_name, "cpu") ||
            !ffCharIsDigit(pathCpuEntry->d_name[strlen("cpu")])) {
            continue;
        }

        ffStrbufAppendS(&path, pathCpuEntry->d_name);
        const char* error = parseCpuCache(&path, result, &buffer, &added);
        if (error) {
            return error;
        }
        ffStrbufSubstrBefore(&path, baseLen);
    }
    return nullptr;
}

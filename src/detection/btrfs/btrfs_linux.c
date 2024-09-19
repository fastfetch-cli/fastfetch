#include "btrfs.h"

#include "common/io/io.h"

enum { uuidLen = (uint32_t) __builtin_strlen("00000000-0000-0000-0000-000000000000") };

static const char* enumerateDevices(FFBtrfsResult* item, FFstrbuf* path, FFstrbuf* buffer)
{
    ffStrbufAppendS(path, "devices/");
    FF_AUTO_CLOSE_DIR DIR* dirp = opendir(path->chars);
    if(dirp == NULL)
        return "opendir(\"/sys/fs/btrfs/UUID/devices\") == NULL";
    const uint32_t devicesPathLen = path->length;

    struct dirent* entry;
    while ((entry = readdir(dirp)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;

        if (item->devices.length)
            ffStrbufAppendC(&item->devices, ',');
        ffStrbufAppendS(&item->devices, entry->d_name);
        ffStrbufAppendS(path, entry->d_name);
        ffStrbufAppendS(path, "/size");

        if (ffReadFileBuffer(path->chars, buffer))
            item->totalSize += ffStrbufToUInt(buffer, 0) * 512;

        ffStrbufSubstrBefore(path, devicesPathLen);
    }

    return NULL;
}

static const char* enumerateFeatures(FFBtrfsResult* item, FFstrbuf* path)
{
    ffStrbufAppendS(path, "features/");
    FF_AUTO_CLOSE_DIR DIR* dirp = opendir(path->chars);
    if(dirp == NULL)
        return "opendir(\"/sys/fs/btrfs/UUID/features\") == NULL";
    const uint32_t featuresPathLen = path->length;

    struct dirent* entry;
    while ((entry = readdir(dirp)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;
        if (item->features.length)
            ffStrbufAppendC(&item->features, ',');
        ffStrbufAppendS(&item->features, entry->d_name);

        ffStrbufSubstrBefore(path, featuresPathLen);
    }

    return NULL;
}

static const char* detectAllocation(FFBtrfsResult* item, FFstrbuf* path, FFstrbuf* buffer)
{
    ffStrbufAppendS(path, "allocation/");
    const uint32_t AllocationPathLen = path->length;

    ffStrbufAppendS(path, "global_rsv_size");
    if (ffReadFileBuffer(path->chars, buffer))
        item->globalReservationTotal = ffStrbufToUInt(buffer, 0);
    else
        return "ffReadFileBuffer(\"/sys/fs/btrfs/UUID/allocation/global_rsv_size\") == NULL";
    ffStrbufSubstrBefore(path, AllocationPathLen);

    ffStrbufAppendS(path, "global_rsv_reserved");
    if (ffReadFileBuffer(path->chars, buffer))
        item->globalReservationUsed = ffStrbufToUInt(buffer, 0);
    ffStrbufSubstrBefore(path, AllocationPathLen);
    item->globalReservationUsed = item->globalReservationTotal - item->globalReservationUsed;

    #define FF_BTRFS_DETECT_TYPE(index, _type) \
    ffStrbufAppendS(path, #_type "/total_bytes"); \
    if (ffReadFileBuffer(path->chars, buffer)) \
        item->allocation[index].total = ffStrbufToUInt(buffer, 0); \
    ffStrbufSubstrBefore(path, AllocationPathLen); \
    \
    ffStrbufAppendS(path, #_type "/bytes_used"); \
    if (ffReadFileBuffer(path->chars, buffer)) \
        item->allocation[index].used = ffStrbufToUInt(buffer, 0); \
    ffStrbufSubstrBefore(path, AllocationPathLen); \
    \
    ffStrbufAppendS(path, #_type "/dup"); \
    item->allocation[index].dup = ffPathExists(path->chars, FF_PATHTYPE_DIRECTORY); \
    ffStrbufSubstrBefore(path, AllocationPathLen); \
    \
    item->allocation[index].type = #_type;

    FF_BTRFS_DETECT_TYPE(0, data);
    FF_BTRFS_DETECT_TYPE(1, metadata);
    FF_BTRFS_DETECT_TYPE(2, system);

    #undef FF_BTRFS_DETECT_TYPE

    return NULL;
}

const char* ffDetectBtrfs(FFlist* result)
{
    FF_STRBUF_AUTO_DESTROY path = ffStrbufCreateS("/sys/fs/btrfs/");
    const uint32_t basePathLen = path.length;

    FF_AUTO_CLOSE_DIR DIR* dirp = opendir(path.chars);
    if(dirp == NULL)
        return "opendir(\"/sys/fs/btrfs\") == NULL";

    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();

    struct dirent* entry;
    while ((entry = readdir(dirp)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;
        if (strlen(entry->d_name) != uuidLen)
            continue;

        FFBtrfsResult* item = ffListAdd(result);
        (*item) = (FFBtrfsResult){};
        ffStrbufInitNS(&item->uuid, uuidLen, entry->d_name);
        ffStrbufInit(&item->name);
        ffStrbufInit(&item->devices);
        ffStrbufInit(&item->features);

        ffStrbufAppendNS(&path, uuidLen, entry->d_name);
        ffStrbufAppendC(&path, '/');
        const uint32_t itemPathLen = path.length;

        ffStrbufAppendS(&path, "label");
        if (ffAppendFileBuffer(path.chars, &item->name))
            ffStrbufTrimRightSpace(&item->name);
        ffStrbufSubstrBefore(&path, itemPathLen);

        enumerateDevices(item, &path, &buffer);
        ffStrbufSubstrBefore(&path, itemPathLen);

        enumerateFeatures(item, &path);
        ffStrbufSubstrBefore(&path, itemPathLen);

        ffStrbufAppendS(&path, "generation");
        if (ffReadFileBuffer(path.chars, &buffer))
            item->generation = (uint32_t) ffStrbufToUInt(&buffer, 0);
        ffStrbufSubstrBefore(&path, itemPathLen);

        ffStrbufAppendS(&path, "nodesize");
        if (ffReadFileBuffer(path.chars, &buffer))
            item->nodeSize = (uint32_t) ffStrbufToUInt(&buffer, 0);
        ffStrbufSubstrBefore(&path, itemPathLen);

        ffStrbufAppendS(&path, "sectorsize");
        if (ffReadFileBuffer(path.chars, &buffer))
            item->sectorSize = (uint32_t) ffStrbufToUInt(&buffer, 0);
        ffStrbufSubstrBefore(&path, itemPathLen);

        detectAllocation(item, &path, &buffer);

        // finally
        ffStrbufSubstrBefore(&path, basePathLen);
    }

    return NULL;
}

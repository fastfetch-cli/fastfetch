#include "btrfs.h"

#include "common/io/io.h"
#include <fcntl.h>

enum { uuidLen = (uint32_t) __builtin_strlen("00000000-0000-0000-0000-000000000000") };

static const char* enumerateDevices(FFBtrfsResult* item, int dfd, FFstrbuf* buffer)
{
    int subfd = openat(dfd, "devices", O_RDONLY | O_CLOEXEC | O_DIRECTORY);
    if (subfd < 0) return "openat(\"/sys/fs/btrfs/UUID/devices\") == -1";

    FF_AUTO_CLOSE_DIR DIR* dirp = fdopendir(subfd);
    if(dirp == NULL)
        return "fdopendir(\"/sys/fs/btrfs/UUID/devices\") == NULL";

    struct dirent* entry;
    while ((entry = readdir(dirp)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;

        if (item->devices.length)
            ffStrbufAppendC(&item->devices, ',');
        ffStrbufAppendS(&item->devices, entry->d_name);

        char path[ARRAY_SIZE(entry->d_name) + ARRAY_SIZE("/size") + 1];
        snprintf(path, ARRAY_SIZE(path), "%s/size", entry->d_name);

        if (ffReadFileBufferRelative(subfd, path, buffer))
            item->totalSize += ffStrbufToUInt(buffer, 0) * 512;
    }

    return NULL;
}

static const char* enumerateFeatures(FFBtrfsResult* item, int dfd)
{
    int subfd = openat(dfd, "features", O_RDONLY | O_CLOEXEC | O_DIRECTORY);
    if (subfd < 0) return "openat(\"/sys/fs/btrfs/UUID/features\") == -1";

    FF_AUTO_CLOSE_DIR DIR* dirp = fdopendir(subfd);
    if(dirp == NULL)
        return "fdopendir(\"/sys/fs/btrfs/UUID/features\") == NULL";

    struct dirent* entry;
    while ((entry = readdir(dirp)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;
        if (item->features.length)
            ffStrbufAppendC(&item->features, ',');
        ffStrbufAppendS(&item->features, entry->d_name);
    }

    return NULL;
}

static const char* detectAllocation(FFBtrfsResult* item, int dfd, FFstrbuf* buffer)
{
    FF_AUTO_CLOSE_FD int subfd = openat(dfd, "allocation", O_RDONLY | O_CLOEXEC | O_PATH | O_DIRECTORY);
    if (subfd < 0) return "openat(\"/sys/fs/btrfs/UUID/allocation\") == -1";

    if (ffReadFileBufferRelative(subfd, "global_rsv_size", buffer))
        item->globalReservationTotal = ffStrbufToUInt(buffer, 0);
    else
        return "ffReadFileBuffer(\"/sys/fs/btrfs/UUID/allocation/global_rsv_size\") == NULL";

    if (ffReadFileBufferRelative(subfd, "global_rsv_reserved", buffer))
        item->globalReservationUsed = ffStrbufToUInt(buffer, 0);
    item->globalReservationUsed = item->globalReservationTotal - item->globalReservationUsed;

    #define FF_BTRFS_DETECT_TYPE(index, _type) \
    if (ffReadFileBufferRelative(subfd, #_type "/total_bytes", buffer)) \
        item->allocation[index].total = ffStrbufToUInt(buffer, 0); \
    \
    if (ffReadFileBufferRelative(subfd, #_type "/bytes_used", buffer)) \
        item->allocation[index].used = ffStrbufToUInt(buffer, 0); \
    \
    item->allocation[index].dup = faccessat(subfd, #_type "/dup/", F_OK, 0) == 0; \
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
    FF_AUTO_CLOSE_DIR DIR* dirp = opendir("/sys/fs/btrfs/");
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
        (*item) = (FFBtrfsResult){
            .uuid = ffStrbufCreateNS(uuidLen, entry->d_name),
            .name = ffStrbufCreate(),
            .devices = ffStrbufCreate(),
            .features = ffStrbufCreate(),
        };

        FF_AUTO_CLOSE_FD int dfd = openat(dirfd(dirp), entry->d_name, O_RDONLY | O_CLOEXEC | O_PATH | O_DIRECTORY);
        if (dfd < 0) continue;

        if (ffAppendFileBufferRelative(dfd, "label", &item->name))
            ffStrbufTrimRightSpace(&item->name);

        enumerateDevices(item, dfd, &buffer);

        enumerateFeatures(item, dfd);

        if (ffReadFileBufferRelative(dfd, "generation", &buffer))
            item->generation = (uint32_t) ffStrbufToUInt(&buffer, 0);

        if (ffReadFileBufferRelative(dfd, "nodesize", &buffer))
            item->nodeSize = (uint32_t) ffStrbufToUInt(&buffer, 0);

        if (ffReadFileBufferRelative(dfd, "sectorsize", &buffer))
            item->sectorSize = (uint32_t) ffStrbufToUInt(&buffer, 0);

        detectAllocation(item, dfd, &buffer);
    }

    return NULL;
}

#include "disk.h"
#include "util/mallocHelper.h"
#include "util/stringUtils.h"

#include <sys/mount.h>
#include <sys/stat.h>

#ifdef __NetBSD__
#include <sys/types.h>
#include <sys/statvfs.h>
#define statfs statvfs
#define f_flags f_flag
#define f_bsize f_frsize
#endif

#ifdef __FreeBSD__
#if __has_include(<libgeom.h>)
#include <libgeom.h>

static const char* detectFsLabel(struct statfs* fs, FFDisk* disk)
{
    if (!ffStrStartsWith(fs->f_mntfromname, "/dev/"))
        return "Only block devices are supported";

    // Detect volume label in geom tree
    static struct gmesh geomTree;
    static struct gclass* cLabels;
    if (!cLabels)
    {
        if (geomTree.lg_ident)
            return "Previous geom_gettree() failed";

        if (geom_gettree(&geomTree) < 0)
        {
            geomTree.lg_ident = (void*)(intptr_t)-1;
            return "geom_gettree() failed";
        }

        for (cLabels = geomTree.lg_class.lh_first; cLabels && !ffStrEquals(cLabels->lg_name, "LABEL"); cLabels = cLabels->lg_class.le_next);
        if (!cLabels)
            return "Class LABEL is not found";
    }

    for (struct ggeom* label = cLabels->lg_geom.lh_first; label; label = label->lg_geom.le_next)
    {
        struct gprovider* provider = label->lg_provider.lh_first;
        if (!provider || !ffStrEquals(label->lg_name, fs->f_mntfromname + strlen("/dev/"))) continue;
        const char* str = strchr(provider->lg_name, '/');
        ffStrbufSetS(&disk->name, str ? str + 1 : provider->lg_name);
    }

    return NULL;
}
#else
static const char* detectFsLabel(FF_MAYBE_UNUSED struct statfs* fs, FF_MAYBE_UNUSED FFDisk* disk)
{
    return "Fastfetch was compiled without libgeom support";
}
#endif

static void detectFsInfo(struct statfs* fs, FFDisk* disk)
{
    if(ffStrbufEqualS(&disk->filesystem, "zfs"))
    {
        disk->type = !ffStrbufStartsWithS(&disk->mountFrom, "zroot/") || ffStrbufStartsWithS(&disk->mountFrom, "zroot/ROOT/")
            ? FF_DISK_VOLUME_TYPE_REGULAR_BIT
            : FF_DISK_VOLUME_TYPE_SUBVOLUME_BIT;
    }
    else if(fs->f_flags & MNT_IGNORE)
        disk->type = FF_DISK_VOLUME_TYPE_HIDDEN_BIT;
    else if(!(fs->f_flags & MNT_LOCAL))
        disk->type = FF_DISK_VOLUME_TYPE_EXTERNAL_BIT;
    else
        disk->type = FF_DISK_VOLUME_TYPE_REGULAR_BIT;

    detectFsLabel(fs, disk);
}
#elif __APPLE__
#include "util/apple/cf_helpers.h"

#include <sys/attr.h>
#include <unistd.h>

#ifndef MAC_OS_X_VERSION_10_15
    #define MNT_REMOVABLE 0x00000200
#endif

struct CmnAttrBuf {
    uint32_t       length;
    attrreference_t nameRef;
    char            nameSpace[NAME_MAX * 3 + 1];
} __attribute__((aligned(4), packed));

void detectFsInfo(struct statfs* fs, FFDisk* disk)
{
    if(fs->f_flags & MNT_DONTBROWSE)
        disk->type = FF_DISK_VOLUME_TYPE_HIDDEN_BIT;
    else if(fs->f_flags & MNT_REMOVABLE || !(fs->f_flags & MNT_LOCAL))
        disk->type = FF_DISK_VOLUME_TYPE_EXTERNAL_BIT;
    else
        disk->type = FF_DISK_VOLUME_TYPE_REGULAR_BIT;

    struct CmnAttrBuf attrBuf;
    if (getattrlist(disk->mountpoint.chars, &(struct attrlist) {
        .bitmapcount = ATTR_BIT_MAP_COUNT,
        .commonattr = ATTR_CMN_NAME,
    }, &attrBuf, sizeof(attrBuf), 0) == 0)
        ffStrbufInitNS(&disk->name, attrBuf.nameRef.attr_length - 1 /* excluding '\0' */, attrBuf.nameSpace);
}
#else
static void detectFsInfo(struct statfs* fs, FFDisk* disk)
{
    #ifdef MNT_IGNORE
    if(fs->f_flags & MNT_IGNORE)
        disk->type = FF_DISK_VOLUME_TYPE_HIDDEN_BIT;
    else
    #endif
    if(!(fs->f_flags & MNT_LOCAL))
        disk->type = FF_DISK_VOLUME_TYPE_EXTERNAL_BIT;
    else
        disk->type = FF_DISK_VOLUME_TYPE_REGULAR_BIT;
}
#endif

const char* ffDetectDisksImpl(FFDiskOptions* options, FFlist* disks)
{
    #ifndef __NetBSD__
    int size = getfsstat(NULL, 0, MNT_WAIT);
    if(size <= 0) return "getfsstat(NULL, 0, MNT_WAIT) failed";
    #else
    int size = getvfsstat(NULL, 0, ST_WAIT);
    if(size <= 0) return "getvfsstat(NULL, 0, ST_WAIT) failed";
    #endif

    FF_AUTO_FREE struct statfs* buf = malloc(sizeof(*buf) * (unsigned) size);
    #ifndef __NetBSD__
    if(getfsstat(buf, (int) (sizeof(*buf) * (unsigned) size), MNT_NOWAIT) <= 0)
        return "getfsstat(buf, size, MNT_NOWAIT) failed";
    #else
    if(getvfsstat(buf, sizeof(*buf) * (unsigned) size, ST_NOWAIT) <= 0)
        return "getvfsstat(buf, size, ST_NOWAIT) failed";
    #endif

    for(struct statfs* fs = buf; fs < buf + size; ++fs)
    {
        if(__builtin_expect(options->folders.length > 0, 0))
        {
            if(!ffStrbufSeparatedContainS(&options->folders, fs->f_mntonname, FF_DISK_FOLDER_SEPARATOR))
                continue;
        }
        else if(!ffStrEquals(fs->f_mntonname, "/") && !ffStrStartsWith(fs->f_mntfromname, "/dev/") && !ffStrEquals(fs->f_fstypename, "zfs") && !ffStrEquals(fs->f_fstypename, "fusefs.sshfs"))
            continue;

        #ifdef __FreeBSD__
        // f_bavail and f_ffree are signed on FreeBSD...
        if(fs->f_bavail < 0) fs->f_bavail = 0;
        if(fs->f_ffree < 0) fs->f_ffree = 0;
        #endif

        FFDisk* disk = ffListAdd(disks);

        disk->bytesTotal = (uint64_t)fs->f_blocks * (uint64_t)fs->f_bsize;
        disk->bytesFree = (uint64_t)fs->f_bfree * (uint64_t)fs->f_bsize;
        disk->bytesAvailable = (uint64_t)fs->f_bavail * (uint64_t)fs->f_bsize;
        disk->bytesUsed = 0; // To be filled in ./disk.c

        disk->filesTotal = (uint32_t) fs->f_files;
        disk->filesUsed = (uint32_t) fs->f_files - (uint32_t) fs->f_ffree;

        ffStrbufInitS(&disk->mountFrom, fs->f_mntfromname);
        ffStrbufInitS(&disk->mountpoint, fs->f_mntonname);
        ffStrbufInitS(&disk->filesystem, fs->f_fstypename);
        ffStrbufInit(&disk->name);
        disk->type = 0;
        disk->createTime = 0;

        detectFsInfo(fs, disk);

        if(fs->f_flags & MNT_RDONLY)
            disk->type |= FF_DISK_VOLUME_TYPE_READONLY_BIT;

        #ifdef __OpenBSD__
        #define st_birthtimespec __st_birthtim
        #endif
        #ifndef __DragonFly__
        struct stat st;
        if(stat(fs->f_mntonname, &st) == 0 && st.st_birthtimespec.tv_sec > 0)
            disk->createTime = (uint64_t)(((uint64_t) st.st_birthtimespec.tv_sec * 1000) + ((uint64_t) st.st_birthtimespec.tv_nsec / 1000000));
        #endif
    }

    return NULL;
}

#include "disk.h"
#include "util/mallocHelper.h"

#include <sys/mount.h>

#ifdef __FreeBSD__
#include "util/stringUtils.h"

static void detectFsInfo(struct statfs* fs, FFDisk* disk)
{
    if(ffStrbufEqualS(&disk->filesystem, "zfs"))
    {
        disk->type = !ffStrStartsWith(fs->f_mntfromname, "zroot/") || ffStrStartsWith(fs->f_mntfromname, "zroot/ROOT/")
            ? FF_DISK_VOLUME_TYPE_REGULAR_BIT
            : FF_DISK_VOLUME_TYPE_SUBVOLUME_BIT;
    }
    else if(!ffStrStartsWith(fs->f_mntfromname, "/dev/"))
        disk->type = FF_DISK_VOLUME_TYPE_HIDDEN_BIT;
    else if(!(fs->f_flags & MNT_LOCAL))
        disk->type = FF_DISK_VOLUME_TYPE_EXTERNAL_BIT;
    else
        disk->type = FF_DISK_VOLUME_TYPE_REGULAR_BIT;
}
#elif __APPLE__
#include <sys/attr.h>
#include <unistd.h>

struct VolAttrBuf {
    uint32_t       length;
    attrreference_t volNameRef;
    char            volNameSpace[MAXPATHLEN];
} __attribute__((aligned(4), packed));

void detectFsInfo(struct statfs* fs, FFDisk* disk)
{
    if(fs->f_flags & MNT_DONTBROWSE)
        disk->type = FF_DISK_VOLUME_TYPE_HIDDEN_BIT;
    else if(fs->f_flags & MNT_REMOVABLE)
        disk->type = FF_DISK_VOLUME_TYPE_EXTERNAL_BIT;
    else
        disk->type = FF_DISK_VOLUME_TYPE_REGULAR_BIT;

    struct VolAttrBuf attrBuf;
    if (getattrlist(fs->f_mntonname, &(struct attrlist) {
        .bitmapcount = ATTR_BIT_MAP_COUNT,
        .volattr = ATTR_VOL_INFO | ATTR_VOL_NAME,
    }, &attrBuf, sizeof(attrBuf), 0) == 0)
        ffStrbufInitNS(&disk->name, attrBuf.volNameRef.attr_length - 1 /* excluding '\0' */, attrBuf.volNameSpace);
}
#endif

const char* ffDetectDisksImpl(FFlist* disks)
{
    int size = getfsstat(NULL, 0, MNT_WAIT);

    if(size <= 0)
        return "getfsstat(NULL, 0, MNT_WAIT) failed";

    FF_AUTO_FREE struct statfs* buf = malloc(sizeof(*buf) * (unsigned) size);
    if(getfsstat(buf, (int) (sizeof(*buf) * (unsigned) size), MNT_NOWAIT) <= 0)
        return "getfsstat(buf, size, MNT_NOWAIT) failed";

    for(struct statfs* fs = buf; fs < buf + size; ++fs)
    {
        FFDisk* disk = ffListAdd(disks);

        #ifdef __FreeBSD__
        // f_bavail and f_ffree are signed on FreeBSD...
        if(fs->f_bavail < 0) fs->f_bavail = 0;
        if(fs->f_ffree < 0) fs->f_ffree = 0;
        #endif

        disk->bytesTotal = fs->f_blocks * fs->f_bsize;
        disk->bytesFree = (uint64_t)fs->f_bfree * fs->f_bsize;
        disk->bytesAvailable = (uint64_t)fs->f_bavail * fs->f_bsize;
        disk->bytesUsed = 0; // To be filled in ./disk.c

        disk->filesTotal = (uint32_t) fs->f_files;
        disk->filesUsed = (uint32_t) (disk->filesTotal - (uint64_t)fs->f_ffree);

        ffStrbufInitS(&disk->mountFrom, fs->f_mntfromname);
        ffStrbufInitS(&disk->mountpoint, fs->f_mntonname);
        ffStrbufInitS(&disk->filesystem, fs->f_fstypename);
        ffStrbufInit(&disk->name);
        disk->type = 0;
        detectFsInfo(fs, disk);

        if(fs->f_flags & MNT_RDONLY)
            disk->type |= FF_DISK_VOLUME_TYPE_READONLY_BIT;
    }

    return NULL;
}

#include "disk.h"

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

    ffStrbufInit(&disk->name);
}
#else
void detectFsInfo(struct statfs* fs, FFDisk* disk);
#endif

const char* ffDetectDisksImpl(FFlist* disks)
{
    struct statfs* buf;

    int size = getmntinfo(&buf, MNT_WAIT);
    if(size <= 0)
        return "getmntinfo(&buf, MNT_WAIT) failed";

    for(struct statfs* fs = buf; fs < buf + size; ++fs)
    {
        FFDisk* disk = ffListAdd(disks);

        #ifdef __FreeBSD__
        // f_bavail and f_ffree are signed on FreeBSD...
        if(fs->f_bavail < 0) fs->f_bavail = 0;
        if(fs->f_ffree < 0) fs->f_ffree = 0;
        #endif

        disk->bytesTotal = fs->f_blocks * fs->f_bsize;
        disk->bytesUsed = disk->bytesTotal - ((uint64_t)fs->f_bfree * fs->f_bsize);

        disk->filesTotal = (uint32_t) fs->f_files;
        disk->filesUsed = (uint32_t) (disk->filesTotal - (uint64_t)fs->f_ffree);

        ffStrbufInitS(&disk->mountpoint, fs->f_mntonname);
        ffStrbufInitS(&disk->filesystem, fs->f_fstypename);
        detectFsInfo(fs, disk);
    }

    return NULL;
}

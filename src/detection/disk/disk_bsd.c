#include "disk.h"

#include <sys/mount.h>

#ifdef __FreeBSD__
static void detectFsInfo(struct statfs* fs, FFDisk* disk)
{
    if(
        ffStrbufStartsWithS(&disk->mountpoint, "/boot") ||
        ffStrbufStartsWithS(&disk->mountpoint, "/dev") ||
        ffStrbufStartsWithS(&disk->mountpoint, "/var") ||
        ffStrbufStartsWithS(&disk->mountpoint, "/tmp") ||
        ffStrbufStartsWithS(&disk->mountpoint, "/proc") ||
        ffStrbufStartsWithS(&disk->mountpoint, "/zroot")
    )
        disk->type = FF_DISK_TYPE_HIDDEN;
    else if((fs->f_flags & MNT_NOSUID) || !(fs->f_flags & MNT_LOCAL))
        disk->type = FF_DISK_TYPE_EXTERNAL;
    else
        disk->type = FF_DISK_TYPE_REGULAR;

    ffStrbufInit(&disk->name);
}
#else
void detectFsInfo(struct statfs* fs, FFDisk* disk);
#endif

void ffDetectDisksImpl(FFDiskResult* disks)
{
    struct statfs* buf;

    int size = getmntinfo(&buf, MNT_WAIT);
    if(size <= 0)
        ffStrbufAppendS(&disks->error, "getmntinfo() failed");

    for(struct statfs* fs = buf; fs < buf + size; ++fs)
    {
        FFDisk* disk = ffListAdd(&disks->disks);

        #ifdef __FreeBSD__
        // f_bavail and f_ffree are signed on FreeBSD...
        if(fs->f_bavail < 0) fs->f_bavail = 0;
        if(fs->f_ffree < 0) fs->f_ffree = 0;
        #endif

        disk->bytesTotal = fs->f_blocks * fs->f_bsize;
        disk->bytesUsed = disk->bytesTotal - ((uint64_t)fs->f_bavail * fs->f_bsize);

        disk->filesTotal = (uint32_t) fs->f_files;
        disk->filesUsed = (uint32_t) (disk->filesTotal - (uint64_t)fs->f_ffree);

        ffStrbufInitS(&disk->mountpoint, fs->f_mntonname);
        ffStrbufInitS(&disk->filesystem, fs->f_fstypename);
        detectFsInfo(fs, disk);
    }
}

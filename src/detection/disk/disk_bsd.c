#include "disk.h"

#include <sys/mount.h>

#ifdef __FreeBSD__
#include <sys/disklabel.h>

static void detectFsInfo(struct statfs* fs, FFDisk* disk)
{
    if(
        ffStrbufStartsWithS(&disk->mountpoint, "/boot") ||
        ffStrbufStartsWithS(&disk->mountpoint, "/dev") ||
        ffStrbufStartsWithS(&disk->mountpoint, "/var") ||
        ffStrbufStartsWithS(&disk->mountpoint, "/tmp") ||
        ffStrbufStartsWithS(&disk->mountpoint, "/proc") ||
        ffStrbufStartsWithS(&disk->mountpoint, "/zroot") ||
        ffStrbufStartsWithS(&disk->mountpoint, "/compat/linux/")
    )
        disk->type = FF_DISK_TYPE_HIDDEN_BIT;
    else if((fs->f_flags & MNT_NOSUID) || !(fs->f_flags & MNT_LOCAL))
        disk->type = FF_DISK_TYPE_EXTERNAL_BIT;
    else
        disk->type = FF_DISK_TYPE_REGULAR_BIT;

    ffStrbufInit(&disk->name);
    struct disklabel* lab = getdiskbyname(fs->f_mntfromname);
    if(lab)
        ffStrbufSetS(&disk->name, lab->d_packname);
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

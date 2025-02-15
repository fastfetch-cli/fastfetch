extern "C"
{
#include "disk.h"
}
#include <fs_info.h>
#include <Directory.h>
#include <Path.h>

const char* ffDetectDisksImpl(FF_MAYBE_UNUSED FFDiskOptions* options, FF_MAYBE_UNUSED FFlist* disks)
{
    int32 pos = 0;

    for (dev_t dev; (dev = next_dev(&pos)) >= B_OK;)
    {
        fs_info fs;
        fs_stat_dev(dev, &fs);

        FFDisk* disk = (FFDisk*) ffListAdd(disks);

        disk->bytesTotal = (uint64_t)fs.total_blocks * (uint64_t) fs.block_size;
        disk->bytesFree = (uint64_t)fs.free_blocks * (uint64_t) fs.block_size;
        disk->bytesAvailable = disk->bytesFree;
        disk->bytesUsed = 0; // To be filled in ./disk. c

        disk->filesTotal = (uint32_t) fs.total_nodes;
        disk->filesUsed = (uint32_t) (disk->filesTotal - (uint64_t)fs.free_nodes);

        ffStrbufInitS(&disk->mountFrom, fs.device_name);
        ffStrbufInit(&disk->mountpoint);
        {
            node_ref node(fs.dev, fs.root);
            BDirectory dir(&node);
            BPath path(&dir);
            if (path.InitCheck() == B_OK)
                ffStrbufSetS(&disk->mountpoint, path.Path());
            else
                ffStrbufSetStatic(&disk->mountpoint, "?");
        }
        ffStrbufInitS(&disk->filesystem, fs.fsh_name);
        ffStrbufInitS(&disk->name, fs.volume_name);
        disk->type = FF_DISK_VOLUME_TYPE_NONE;
        if (!(fs.flags & B_FS_IS_PERSISTENT))
            disk->type = (FFDiskVolumeType) (disk->type | FF_DISK_VOLUME_TYPE_HIDDEN_BIT);
        if (fs.flags & B_FS_IS_READONLY)
            disk->type = (FFDiskVolumeType) (disk->type | FF_DISK_VOLUME_TYPE_READONLY_BIT);
        if (fs.flags & B_FS_IS_REMOVABLE)
            disk->type = (FFDiskVolumeType) (disk->type | FF_DISK_VOLUME_TYPE_EXTERNAL_BIT);
        disk->createTime = 0;
    }
    return 0;
}

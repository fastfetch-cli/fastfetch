#include "disk.h"

#include <sys/mount.h>
#import <Foundation/Foundation.h>

void detectFsInfo(struct statfs* fs, FFDisk* disk)
{
    // FreeBSD doesn't support these flags
    if(fs->f_flags & MNT_DONTBROWSE)
        disk->type = FF_DISK_TYPE_HIDDEN;
    else if(fs->f_flags & MNT_REMOVABLE)
        disk->type = FF_DISK_TYPE_EXTERNAL;
    else
        disk->type = FF_DISK_TYPE_REGULAR;

    ffStrbufInitS(&disk->name, [NSFileManager.defaultManager displayNameAtPath:@(fs->f_mntonname)].UTF8String);
}

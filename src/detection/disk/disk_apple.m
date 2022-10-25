#include "disk.h"

#include <sys/statvfs.h>
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

void ffDetectDisksImpl(FFDiskResult* disks)
{
    NSArray *keys = [NSArray arrayWithObjects:NSURLVolumeNameKey, nil];
    NSArray *urls = [NSFileManager.defaultManager mountedVolumeURLsIncludingResourceValuesForKeys:keys options:0];

    if(urls == nil)
    {
        ffStrbufAppendS(&disks->error, "[NSFileManager.defaultManager mountedVolumeURLsIncludingResourceValuesForKeys] failed");
        return;
    }

    for (NSURL *url in urls)
    {
        FFDisk* disk = ffListAdd(&disks->disks);

        ffStrbufInitS(&disk->mountpoint, [url.relativePath cStringUsingEncoding:NSUTF8StringEncoding]);

        NSString* filesystem;
        BOOL removable;
        [NSWorkspace.sharedWorkspace getFileSystemInfoForPath:url.relativePath
            isRemovable:&removable
            isWritable:nil
            isUnmountable:nil
            description:nil
            type:&filesystem
        ];
        ffStrbufInitS(&disk->filesystem, [filesystem cStringUsingEncoding:NSUTF8StringEncoding]);

        NSError* error;

        NSNumber* isBrowsable;
        if([url getResourceValue:&isBrowsable forKey:NSURLVolumeIsBrowsableKey error:&error] == YES && !isBrowsable.boolValue)
            disk->type = FF_DISK_TYPE_HIDDEN;
        else if(removable)
            disk->type = FF_DISK_TYPE_EXTERNAL;
        else
            disk->type = FF_DISK_TYPE_REGULAR;

        NSString* volumeName;
        if([url getResourceValue:&volumeName forKey:NSURLVolumeNameKey error:&error] == YES)
            ffStrbufInitS(&disk->name, [volumeName cStringUsingEncoding:NSUTF8StringEncoding]);
        else
            ffStrbufInit(&disk->name);

        struct statvfs fs;
        if(statvfs(disk->mountpoint.chars, &fs) != 0)
            memset(&fs, 0, sizeof(struct statvfs)); //Set all values to 0, so our values get initialized to 0 too

        disk->bytesTotal = fs.f_blocks * fs.f_frsize;
        disk->bytesUsed = disk->bytesTotal - (fs.f_bavail * fs.f_frsize);

        disk->filesTotal = (uint32_t) fs.f_files;
        disk->filesUsed = (uint32_t) (disk->filesTotal - fs.f_ffree);
    }
}

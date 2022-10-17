#include "disk.h"

#include <sys/statvfs.h>
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

static bool getBool(NSURL* url, NSURLResourceKey key)
{
    NSError *error;
    NSNumber* result;
    if([url getResourceValue:&result forKey:key error:&error] == NO)
        return false;

    return result.boolValue;
}

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
        [[NSWorkspace sharedWorkspace] getFileSystemInfoForPath:url.relativePath
            isRemovable:nil
            isWritable:nil
            isUnmountable:nil
            description:nil
            type:&filesystem
        ];
        ffStrbufInitS(&disk->filesystem, [filesystem cStringUsingEncoding:NSUTF8StringEncoding]);

        if(getBool(url, NSURLVolumeIsRemovableKey))
            disk->type = FF_DISK_TYPE_EXTERNAL;
        else if(getBool(url, NSURLVolumeIsBrowsableKey))
            disk->type = FF_DISK_TYPE_REGULAR;
        else
            disk->type = FF_DISK_TYPE_HIDDEN;

        struct statvfs fs;
        if(statvfs(disk->mountpoint.chars, &fs) != 0)
            memset(&fs, 0, sizeof(struct statvfs)); //Set all values to 0, so our values get initialized to 0 too

        disk->bytesTotal = fs.f_blocks * fs.f_frsize;
        disk->bytesUsed = disk->bytesTotal - (fs.f_bavail * fs.f_frsize);

        disk->filesTotal = (uint32_t) fs.f_files;
        disk->filesUsed = (uint32_t) (disk->filesTotal - fs.f_ffree);
    }
}

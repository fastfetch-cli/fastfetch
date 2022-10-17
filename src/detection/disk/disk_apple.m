#include "disk.h"

#include <sys/statvfs.h>
#import <Foundation/Foundation.h>

void ffDetectDisksImpl(FFDiskResult* disks)
{
    NSArray *keys = [NSArray arrayWithObjects:NSURLVolumeNameKey, nil];
    NSArray *urls = [NSFileManager.defaultManager mountedVolumeURLsIncludingResourceValuesForKeys:keys
                                                  options:NSVolumeEnumerationSkipHiddenVolumes];

    if(urls == nil)
    {
        ffStrbufAppendS(&disks->error, "[NSFileManager.defaultManager mountedVolumeURLsIncludingResourceValuesForKeys] failed");
        return;
    }

    for (NSURL *url in urls) {
        NSError *error;
        NSNumber* removable;
        if([url getResourceValue:&removable forKey:NSURLVolumeIsRemovableKey error:&error] == NO)
            continue;
        if(removable.boolValue && !instance->config.diskRemovable)
            continue;

        FFDiskResult* folder = (FFDiskResult*)ffListAdd(folders);
        ffDetectDiskWithStatvfs([url.relativePath cStringUsingEncoding:NSUTF8StringEncoding], NULL, folder);
        folder->removable = removable.boolValue;
    }

    return NULL;
}

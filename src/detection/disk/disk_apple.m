#include "disk.h"

#import <Foundation/Foundation.h>

void ffDetectDiskWithStatvfs(const char* folderPath, struct statvfs* fs, FFDiskResult* result);

const char* ffDiskAutodetectFolders(FFinstance* instance, FFlist* folders)
{
    NSArray *keys = [NSArray arrayWithObjects:NSURLVolumeNameKey, nil];
    NSArray *urls = [NSFileManager.defaultManager mountedVolumeURLsIncludingResourceValuesForKeys:keys
                                                  options:NSVolumeEnumerationSkipHiddenVolumes];
    if(urls == nil)
        return "[NSFileManager.defaultManager mountedVolumeURLsIncludingResourceValuesForKeys] failed";

    for (NSURL *url in urls) {
        NSError *error;
        NSNumber* removable;
        if([url getResourceValue:&removable forKey:NSURLVolumeIsRemovableKey error:&error] == NO)
            continue;
        if(removable.boolValue && !instance->config.diskRemovable)
            continue;

        ffDetectDiskWithStatvfs([url.relativePath cStringUsingEncoding:NSUTF8StringEncoding], NULL, (FFDiskResult*)ffListAdd(folders));
    }

    return NULL;
}

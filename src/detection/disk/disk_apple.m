#include "disk.h"

#import <Foundation/Foundation.h>

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

        ffStrbufInitS((FFstrbuf *)ffListAdd(folders), [url.relativePath cStringUsingEncoding:NSUTF8StringEncoding]);
    }

    return NULL;
}

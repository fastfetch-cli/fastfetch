#include "diskio.h"

#include <IOKit/IOKitLib.h>
#include <IOKit/storage/IOMedia.h>
#include <IOKit/storage/IOBlockStorageDriver.h>

const char* ffDiskIOGetIoCounters(FFlist* result, FFDiskIOOptions* options)
{
    io_iterator_t iterator;
    if(IOServiceGetMatchingServices(MACH_PORT_NULL, IOServiceMatching(kIOMediaClass), &iterator) != kIOReturnSuccess)
        return "IOServiceGetMatchingServices() failed";

    io_registry_entry_t registryEntry;
    while((registryEntry = IOIteratorNext(iterator)) != 0)
    {
        CFMutableDictionaryRef properties;
        if(IORegistryEntryCreateCFProperties(registryEntry, &properties, kCFAllocatorDefault, kNilOptions) != kIOReturnSuccess)
        {
            IOObjectRelease(registryEntry);
            continue;
        }

        io_registry_entry_t parent;
        IORegistryEntryGetParentEntry(registryEntry, kIOServicePlane, &parent);
        if(IOObjectConformsTo(parent, kIOBlockStorageDriverClass)) continue;

        io_name_t name;
        if (IORegistryEntryGetName(registryEntry, name) != KERN_SUCCESS)
            continue;

        // NSDictionary* props = (__bridge NSDictionary*) properties;

        // id statistics = [props valueForKey:@(kIOBlockStorageDriverStatisticsKey)];
        // if (!statistics) continue;

        // id volGroupaMntFromName = [props valueForKey:@"VolGroupMntFromName"];
        // if (!volGroupaMntFromName) continue;

        // NSLog(@"%@", props);
    }
}

#include "bios.h"
#include "util/apple/cf_helpers.h"

#include <IOKit/IOKitLib.h>

void ffDetectBios(FFBiosResult* bios)
{
    ffStrbufInit(&bios->error);
    ffStrbufInit(&bios->biosDate);
    ffStrbufInit(&bios->biosRelease);
    ffStrbufInit(&bios->biosVendor);
    ffStrbufInit(&bios->biosVersion);

    io_registry_entry_t registryEntry;

    #ifndef __aarch64__

    //https://github.com/osquery/osquery/blob/master/osquery/tables/system/darwin/smbios_tables.cpp
    //For Intel
    if((registryEntry = IORegistryEntryFromPath(MACH_PORT_NULL, "IODeviceTree:/rom")))
    {
        CFMutableDictionaryRef properties;
        if(IORegistryEntryCreateCFProperties(registryEntry, &properties, kCFAllocatorDefault, kNilOptions) != kIOReturnSuccess)
        {
            IOObjectRelease(registryEntry);
            ffStrbufAppendS(&bios->error, "IORegistryEntryCreateCFProperties(registryEntry) failed");
            return;
        }

        ffCfDictGetString(properties, CFSTR("vendor"), &bios->biosVendor);
        ffCfDictGetString(properties, CFSTR("version"), &bios->biosVersion);
        ffCfDictGetString(properties, CFSTR("release-date"), &bios->biosDate);
        if(!ffStrbufContainC(&bios->biosDate, '-'))
            ffStrbufAppendS(&bios->biosRelease, "Efi-");
        ffStrbufAppend(&bios->biosRelease, &bios->biosVersion);

        CFRelease(properties);
        IOObjectRelease(registryEntry);
        return;
    }

    #else

    //For arm64
    if((registryEntry = IORegistryEntryFromPath(MACH_PORT_NULL, "IODeviceTree:/")))
    {
        CFMutableDictionaryRef properties;
        if(IORegistryEntryCreateCFProperties(registryEntry, &properties, kCFAllocatorDefault, kNilOptions) == kIOReturnSuccess)
        {
            ffCfDictGetString(properties, CFSTR("manufacturer"), &bios->biosVendor);
            ffCfDictGetString(properties, CFSTR("time-stamp"), &bios->date);
            CFRelease(properties);
        }
        IOObjectRelease(registryEntry);
    }

    if((registryEntry = IORegistryEntryFromPath(MACH_PORT_NULL, "IODeviceTree:/chosen")))
    {
        CFMutableDictionaryRef properties;
        if(IORegistryEntryCreateCFProperties(registryEntry, &properties, kCFAllocatorDefault, kNilOptions) == kIOReturnSuccess)
        {
            ffCfDictGetString(properties, CFSTR("system-firmware-version"), &bios->biosRelease);
            ffStrbufAppend(&bios->biosVersion, &bios->biosRelease);
            ffStrbufSubstrAfterFirstC(&bios->biosVersion, '-');
            CFRelease(properties);
        }
        IOObjectRelease(registryEntry);
    }

    #endif
}

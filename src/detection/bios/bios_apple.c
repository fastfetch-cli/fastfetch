#include "bios.h"
#include "common/apple/cf_helpers.h"

#include <IOKit/IOKitLib.h>

const char* ffDetectBios(FFBiosResult* bios)
{
    #ifndef __aarch64__

    //https://github.com/osquery/osquery/blob/master/osquery/tables/system/darwin/smbios_tables.cpp
    //For Intel
    FF_IOOBJECT_AUTO_RELEASE io_registry_entry_t deviceRom = IORegistryEntryFromPath(MACH_PORT_NULL, "IODeviceTree:/rom");
    if (!deviceRom)
        return "IODeviceTree:/rom not found";

    FF_CFTYPE_AUTO_RELEASE CFMutableDictionaryRef deviceRomProps = NULL;
    if(IORegistryEntryCreateCFProperties(deviceRom, &deviceRomProps, kCFAllocatorDefault, kNilOptions) != kIOReturnSuccess)
        return "IORegistryEntryCreateCFProperties(deviceRom) failed";

    ffCfDictGetString(deviceRomProps, CFSTR("vendor"), &bios->vendor);
    ffCfDictGetString(deviceRomProps, CFSTR("version"), &bios->version);
    ffCfDictGetString(deviceRomProps, CFSTR("release-date"), &bios->date);
    ffStrbufSetStatic(&bios->type, "UEFI");

    #else

    //For arm64
    FF_IOOBJECT_AUTO_RELEASE io_registry_entry_t device = IORegistryEntryFromPath(MACH_PORT_NULL, "IODeviceTree:/");
    if (!device)
        return "IODeviceTree:/ not found";

    FF_CFTYPE_AUTO_RELEASE CFMutableDictionaryRef deviceProps = NULL;
    if(IORegistryEntryCreateCFProperties(device, &deviceProps, kCFAllocatorDefault, kNilOptions) != kIOReturnSuccess)
        return "IORegistryEntryCreateCFProperties(device) failed";

    ffCfDictGetString(deviceProps, CFSTR("manufacturer"), &bios->vendor);
    ffCfDictGetString(deviceProps, CFSTR("time-stamp"), &bios->date);

    FF_IOOBJECT_AUTO_RELEASE io_registry_entry_t deviceChosen = IORegistryEntryFromPath(MACH_PORT_NULL, "IODeviceTree:/chosen");
    if (deviceChosen)
    {
        FF_CFTYPE_AUTO_RELEASE CFStringRef systemFirmWareVersion = IORegistryEntryCreateCFProperty(deviceChosen, CFSTR("system-firmware-version"), kCFAllocatorDefault, kNilOptions);
        if (systemFirmWareVersion)
        {
            ffCfStrGetString(systemFirmWareVersion, &bios->version);
            uint32_t index = ffStrbufFirstIndexC(&bios->version, '-');
            if (index != bios->version.length)
            {
                ffStrbufAppendNS(&bios->type, index, bios->version.chars);
                ffStrbufRemoveSubstr(&bios->version, 0, index + 1);
            }
        }
    }
    if (!bios->type.length)
        ffStrbufSetStatic(&bios->type, "iBoot");
    #endif

    return NULL;
}

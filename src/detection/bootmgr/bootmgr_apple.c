#include "bootmgr.h"
#include "common/io.h"
#include "common/apple/cf_helpers.h"

#include <IOKit/IOKitLib.h>

static const char* detectSecureBoot(bool* result)
{
    #if __aarch64__
    FF_IOOBJECT_AUTO_RELEASE io_registry_entry_t entryDevice = IORegistryEntryFromPath(MACH_PORT_NULL, "IODeviceTree:/chosen");
    if (!entryDevice)
        return "IORegistryEntryFromPath() failed";

    FF_CFTYPE_AUTO_RELEASE CFTypeRef prop = IORegistryEntryCreateCFProperty(entryDevice, CFSTR("secure-boot"), kCFAllocatorDefault, 0);
    if (!prop)
        return "IORegistryEntryCreateCFProperty() failed";

    if (CFGetTypeID(prop) != CFDataGetTypeID() || CFDataGetLength((CFDataRef) prop) == 0)
        return "Invalid secure-boot property";

    *result = (bool) *CFDataGetBytePtr((CFDataRef) prop);
    #else
    FF_IOOBJECT_AUTO_RELEASE io_registry_entry_t entryDevice = IORegistryEntryFromPath(MACH_PORT_NULL, "IODeviceTree:/options");
    if (!entryDevice)
        return "IORegistryEntryFromPath() failed";

    FF_CFTYPE_AUTO_RELEASE CFTypeRef prop = IORegistryEntryCreateCFProperty(entryDevice, CFSTR("94b73556-2197-4702-82a8-3e1337dafbfb:AppleSecureBootPolicy"), kCFAllocatorDefault, 0);
    if (!prop)
        return "IORegistryEntryCreateCFProperty() failed";

    if (CFGetTypeID(prop) != CFDataGetTypeID() || CFDataGetLength((CFDataRef) prop) == 0)
        return "Invalid secure-boot property";

    *result = *CFDataGetBytePtr((CFDataRef) prop) != 0x02 /* Permissive Security */;
    #endif

    return NULL;
}

const char* ffDetectBootmgr(FFBootmgrResult* result)
{
    if (ffPathExists("/System/Library/CoreServices/boot.efi", FF_PATHTYPE_FILE))
        ffStrbufSetStatic(&result->firmware, "/System/Library/CoreServices/boot.efi");

    ffStrbufSetStatic(&result->name, "iBoot");

    detectSecureBoot(&result->secureBoot);

    return NULL;
}

#include "bootmgr.h"
#include "common/io/io.h"
#include "util/apple/cf_helpers.h"

#include <IOKit/IOKitLib.h>

FF_MAYBE_UNUSED static const char* detectFromIokit(bool* secureBoot)
{
    FF_IOOBJECT_AUTO_RELEASE io_registry_entry_t entryDevice = IORegistryEntryFromPath(kIOMainPortDefault, "IODeviceTree:/chosen");
    if (!entryDevice)
        return "IORegistryEntryFromPath() failed";

    FF_CFTYPE_AUTO_RELEASE CFTypeRef prop = IORegistryEntryCreateCFProperty(entryDevice, CFSTR("secure-boot"), kCFAllocatorDefault, 0);
    if (!prop)
        return "IORegistryEntryCreateCFProperty() failed";

    if (CFGetTypeID(prop) != CFDataGetTypeID() || CFDataGetLength((CFDataRef) prop) == 0)
        return "Invalid secure-boot property";

    *secureBoot = (bool) *CFDataGetBytePtr((CFDataRef) prop);
    return NULL;
}

const char* ffDetectBootmgr(FFBootmgrResult* result)
{
    if (ffPathExists("/System/Library/CoreServices/boot.efi", FF_PATHTYPE_FILE))
        ffStrbufSetStatic(&result->firmware, "/System/Library/CoreServices/boot.efi");

    ffStrbufSetStatic(&result->name, "iBoot");

    detectFromIokit(&result->secureBoot);

    return NULL;
}

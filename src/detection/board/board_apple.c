#include "board.h"

#include "common/sysctl.h"
#include "util/apple/cf_helpers.h"

const char* ffDetectBoard(FFBoardResult* result)
{
    FF_IOOBJECT_AUTO_RELEASE io_registry_entry_t service = IOServiceGetMatchingService(MACH_PORT_NULL, IOServiceMatching("IOPlatformExpertDevice"));
    if (!service)
        return "No IOPlatformExpertDevice found";

    io_name_t name;
    if (IORegistryEntryGetName(service, name) == kIOReturnSuccess)
        ffStrbufSetS(&result->name, name);

    FF_CFTYPE_AUTO_RELEASE CFTypeRef manufacturer = IORegistryEntryCreateCFProperty(service, CFSTR("manufacturer"), kCFAllocatorDefault, kNilOptions);
    if (manufacturer)
        ffCfStrGetString(manufacturer, &result->vendor);

    return NULL;
}

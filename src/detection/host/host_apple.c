#include "host.h"
#include "common/sysctl.h"
#include "util/apple/cf_helpers.h"
#include "util/stringUtils.h"

#include <IOKit/IOKitLib.h>

const char* getProductNameWithIokit(FFstrbuf* result)
{
    FF_IOOBJECT_AUTO_RELEASE io_registry_entry_t registryEntry = IOServiceGetMatchingService(MACH_PORT_NULL, IOServiceNameMatching("product"));
    if (!registryEntry)
        return "IOServiceGetMatchingService() failed";

    FF_CFTYPE_AUTO_RELEASE CFStringRef productName = IORegistryEntryCreateCFProperty(registryEntry, CFSTR("product-name"), kCFAllocatorDefault, kNilOptions);
    if (!productName)
        return "IORegistryEntryCreateCFProperty() failed";

    return ffCfStrGetString(productName, result);
}

const char* getOthersByIokit(FFHostResult* host)
{
    FF_IOOBJECT_AUTO_RELEASE io_registry_entry_t registryEntry = IOServiceGetMatchingService(MACH_PORT_NULL, IOServiceMatching("IOPlatformExpertDevice"));
    if (!registryEntry)
        return "IOServiceGetMatchingService() failed";

    FF_CFTYPE_AUTO_RELEASE CFStringRef serialNumber = IORegistryEntryCreateCFProperty(registryEntry, CFSTR(kIOPlatformSerialNumberKey), kCFAllocatorDefault, kNilOptions);
    if (serialNumber)
        ffCfStrGetString(serialNumber, &host->serial);

    FF_CFTYPE_AUTO_RELEASE CFStringRef uuid = IORegistryEntryCreateCFProperty(registryEntry, CFSTR(kIOPlatformUUIDKey), kCFAllocatorDefault, kNilOptions);
    if (uuid)
        ffCfStrGetString(uuid, &host->uuid);

    FF_CFTYPE_AUTO_RELEASE CFStringRef manufacturer = IORegistryEntryCreateCFProperty(registryEntry, CFSTR("manufacturer"), kCFAllocatorDefault, kNilOptions);
    if (manufacturer)
        ffCfStrGetString(manufacturer, &host->vendor);

    FF_CFTYPE_AUTO_RELEASE CFStringRef version = IORegistryEntryCreateCFProperty(registryEntry, CFSTR("version"), kCFAllocatorDefault, kNilOptions);
    if (version)
        ffCfStrGetString(version, &host->version);

    return NULL;
}

const char* ffDetectHost(FFHostResult* host)
{
    const char* error = ffSysctlGetString("hw.model", &host->family);
    if (error) return error;

    ffStrbufSetStatic(&host->name, ffHostGetMacProductNameWithHwModel(&host->family));
    if (host->name.length == 0)
        getProductNameWithIokit(&host->name);
    if (host->name.length == 0)
        ffStrbufSet(&host->name, &host->family);
    getOthersByIokit(host);
    return NULL;
}

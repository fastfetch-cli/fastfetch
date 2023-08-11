#include "monitor.h"
#include "detection/displayserver/displayserver.h"
#include "util/apple/cf_helpers.h"
#include "util/apple/ddcci.h"
#include "util/edidHelper.h"

extern CFDictionaryRef CoreDisplay_DisplayCreateInfoDictionary(CGDirectDisplayID display) __attribute__((weak_import));

static const char* detectWithDisplayServices(const FFDisplayServerResult* displayServer, FFlist* results)
{
    if(!CoreDisplay_DisplayCreateInfoDictionary) return "CoreDisplay_DisplayCreateInfoDictionary is not available";

    FF_LIST_FOR_EACH(FFDisplayResult, display, displayServer->displays)
    {
        if (display->type == FF_DISPLAY_TYPE_BUILTIN)
        {
            CFDictionaryRef FF_CFTYPE_AUTO_RELEASE displayInfo = CoreDisplay_DisplayCreateInfoDictionary((CGDirectDisplayID) display->id);
            if(displayInfo)
            {
                int width, height;
                if (ffCfDictGetInt(displayInfo, CFSTR("kCGDisplayPixelWidth"), &width) || // Default resolution (limited by connectors, GPUs, etc.)
                    ffCfDictGetInt(displayInfo, CFSTR("kCGDisplayPixelHeight"), &height) ||
                    width <= 0 || height <= 0)
                    continue;

                FFMonitorResult* monitor = (FFMonitorResult*) ffListAdd(results);
                monitor->width = (uint32_t) width;
                monitor->height = (uint32_t) height;
                ffStrbufInitCopy(&monitor->name, &display->name);

                CGSize size = CGDisplayScreenSize((CGDirectDisplayID) display->id);
                monitor->physicalWidth = (uint32_t) (size.width + 0.5);
                monitor->physicalHeight = (uint32_t) (size.height + 0.5);
            }
        }
    }
    return NULL;
}

static const char* detectWithDdcci(FFlist* results)
{
    if (!IOAVServiceCreate || !IOAVServiceReadI2C)
        return "IOAVService is not available";

    CFMutableDictionaryRef matchDict = IOServiceMatching("DCPAVServiceProxy");
    if (matchDict == NULL)
        return "IOServiceMatching(\"DCPAVServiceProxy\") failed";

    io_iterator_t iterator;
    if(IOServiceGetMatchingServices(MACH_PORT_NULL, matchDict, &iterator) != kIOReturnSuccess)
        return "IOServiceGetMatchingServices() failed";

    FF_STRBUF_AUTO_DESTROY location = ffStrbufCreate();

    io_registry_entry_t registryEntry;
    while((registryEntry = IOIteratorNext(iterator)) != 0)
    {
        CFMutableDictionaryRef properties;
        if(IORegistryEntryCreateCFProperties(registryEntry, &properties, kCFAllocatorDefault, kNilOptions) != kIOReturnSuccess)
        {
            IOObjectRelease(registryEntry);
            continue;
        }

        ffStrbufClear(&location);
        if(ffCfDictGetString(properties, CFSTR("Location"), &location) || ffStrbufEqualS(&location, "Embedded"))
        {
            // Builtin display should be handled by DisplayServices
            IOObjectRelease(registryEntry);
            continue;
        }

        FF_CFTYPE_AUTO_RELEASE IOAVServiceRef service = IOAVServiceCreateWithService(kCFAllocatorDefault, (io_service_t) registryEntry);
        IOObjectRelease(registryEntry);

        if (!service) continue;

        FF_CFTYPE_AUTO_RELEASE CFDataRef edid = NULL;
        if (IOAVServiceCopyEDID(service, &edid) != KERN_SUCCESS)
            continue;

        if (CFDataGetLength(edid) < 128)
            continue;

        uint32_t width, height;
        const uint8_t* edidData = CFDataGetBytePtr(edid);
        ffEdidGetPhysicalResolution(edidData, &width, &height);
        if (width == 0 || height == 0) continue;

        FFMonitorResult* display = (FFMonitorResult*) ffListAdd(results);
        display->width = width;
        display->height = height;
        ffStrbufInit(&display->name);
        ffEdidGetName(edidData, &display->name);
        ffEdidGetPhysicalSize(edidData, &display->physicalWidth, &display->physicalHeight);
    }
    return NULL;
}

const char* ffDetectMonitor(FFlist* results)
{
    const FFDisplayServerResult* displayServer = ffConnectDisplayServer();

    detectWithDisplayServices(displayServer, results);

    if (displayServer->displays.length > results->length)
        detectWithDdcci(results);
    return NULL;
}

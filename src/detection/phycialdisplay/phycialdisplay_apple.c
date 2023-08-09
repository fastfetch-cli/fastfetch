#include "phycialdisplay.h"
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
        if (display->type == FF_DISPLAY_TYPE_BUILTIN || display->type == FF_DISPLAY_TYPE_UNKNOWN)
        {
            CFDictionaryRef FF_CFTYPE_AUTO_RELEASE displayInfo = CoreDisplay_DisplayCreateInfoDictionary((CGDirectDisplayID) display->id);
            if(displayInfo)
            {
                int width, height;
                if (ffCfDictGetInt(displayInfo, CFSTR("kCGDisplayPixelWidth"), &width) ||
                    ffCfDictGetInt(displayInfo, CFSTR("kCGDisplayPixelHeight"), &height) ||
                    width <= 0 || height <= 0)
                    continue;

                FFPhycialDisplayResult* display = (FFPhycialDisplayResult*) ffListAdd(results);
                display->width = (uint32_t) width;
                display->height = (uint32_t) height;
                ffStrbufInit(&display->name);

                CFDictionaryRef productNames;
                if(!ffCfDictGetDict(displayInfo, CFSTR(kDisplayProductName), &productNames))
                    ffCfDictGetString(productNames, CFSTR("en_US"), &display->name);
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
        if (IOAVServiceCopyEDID(service, &edid) != KERN_SUCCESS )
            continue;

        if (CFDataGetLength(edid) < 128)
            continue;

        uint32_t width, height;
        ffEdidGetPhycialDisplay(CFDataGetBytePtr(edid), &width, &height);
        if (width == 0 || height == 0) continue;

        FFPhycialDisplayResult* display = (FFPhycialDisplayResult*) ffListAdd(results);
        display->width = width;
        display->height = height;
        ffStrbufInit(&display->name);
        ffEdidGetName(CFDataGetBytePtr(edid), &display->name);
    }
    return NULL;
}

const char* ffDetectPhycialDisplay(FFlist* results)
{
    const FFDisplayServerResult* displayServer = ffConnectDisplayServer();

    detectWithDisplayServices(displayServer, results);

    if (displayServer->displays.length > results->length)
        detectWithDdcci(results);
    return NULL;
}

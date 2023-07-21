#include "brightness.h"
#include "detection/displayserver/displayserver.h"
#include "util/apple/cf_helpers.h"

#include <CoreGraphics/CoreGraphics.h>

extern int DisplayServicesGetBrightness(CGDirectDisplayID display, float *brightness) __attribute__((weak_import));

// DDC/CI
typedef CFTypeRef IOAVServiceRef;
extern IOAVServiceRef IOAVServiceCreate(CFAllocatorRef allocator) __attribute__((weak_import));
extern IOAVServiceRef IOAVServiceCreateWithService(CFAllocatorRef allocator, io_service_t service) __attribute__((weak_import));
extern IOReturn IOAVServiceCopyEDID(IOAVServiceRef service, CFDataRef* x2) __attribute__((weak_import));
extern IOReturn IOAVServiceReadI2C(IOAVServiceRef service, uint32_t chipAddress, uint32_t offset, void* outputBuffer, uint32_t outputBufferSize) __attribute__((weak_import));
extern IOReturn IOAVServiceWriteI2C(IOAVServiceRef service, uint32_t chipAddress, uint32_t dataAddress, void* inputBuffer, uint32_t inputBufferSize) __attribute__((weak_import));

// Works for internal display
static const char* detectWithDisplayServices(const FFDisplayServerResult* displayServer, FFlist* result)
{
    if(DisplayServicesGetBrightness == NULL)
        return "DisplayServices function DisplayServicesGetBrightness is not available";

    FF_LIST_FOR_EACH(FFDisplayResult, display, displayServer->displays)
    {
        float value;
        if(DisplayServicesGetBrightness((CGDirectDisplayID) display->id, &value) == kCGErrorSuccess)
        {
            FFBrightnessResult* brightness = (FFBrightnessResult*) ffListAdd(result);
            brightness->value = value * 100;
            ffStrbufInitCopy(&brightness->name, &display->name);
        }
    }

    return NULL;
}

static void getNameFromEdid(uint8_t edid[128], FFstrbuf* name)
{
    // https://github.com/jinksong/read_edid/blob/master/parse-edid/parse-edid.c
    for (uint32_t i = 0x36; i < 0x7E; i += 0x12)
    { // read through descriptor blocks...
        if (edid[i] == 0x00)
        { // not a timing descriptor
            if (edid[i+3] == 0xfc)
            { // Model Name tag
                for (uint32_t j = 0; j < 13; j++)
                {
                    if (edid[i + 5 + j] == 0x0a)
                        return;
                    ffStrbufAppendC(name, (char) edid[i + 5 + j]);
                }
            }
        }
    }
}

// https://github.com/waydabber/m1ddc
// Works for Apple Silicon and USB-C adapter connection ( but not HTMI )
static const char* detectWithDdcci(FFlist* result)
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

        {
            uint8_t i2cIn[4] = { 0x82, 0x01, 0x10 /* luminance */ };
            i2cIn[3] = 0x6e ^ i2cIn[0] ^ i2cIn[1] ^ i2cIn[2];

            for (uint32_t i = 0; i < 2; ++i)
            {
                IOAVServiceWriteI2C(service, 0x37, 0x51, i2cIn, sizeof(i2cIn));
                usleep(10000);
            }
        }

        uint8_t i2cOut[12] = {};
        if (IOAVServiceReadI2C(service, 0x37, 0x51, i2cOut, sizeof(i2cOut)) == KERN_SUCCESS)
        {
            if (i2cOut[2] != 0x02 || i2cOut[3] != 0x00)
                continue;

            uint32_t current = ((uint32_t) i2cOut[8] << 8u) + (uint32_t) i2cOut[9];
            uint32_t max = ((uint32_t) i2cOut[6] << 8u) + (uint32_t) i2cOut[7];

            FFBrightnessResult* brightness = (FFBrightnessResult*) ffListAdd(result);
            brightness->value = (float) current * 100.f / max;
            ffStrbufInit(&brightness->name);

            uint8_t edid[128] = {};
            if (IOAVServiceReadI2C(service, 0x50, 0x00, edid, sizeof(edid)) == KERN_SUCCESS)
                getNameFromEdid(edid, &brightness->name);
        }
    }

    return NULL;
}

const char* ffDetectBrightness(FFlist* result)
{
    const FFDisplayServerResult* displayServer = ffConnectDisplayServer();
    detectWithDisplayServices(displayServer, result);

    if (instance.config.allowSlowOperations && displayServer->displays.length > result->length)
        detectWithDdcci(result);

    return NULL;
}

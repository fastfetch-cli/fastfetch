#include "brightness.h"
#include "detection/displayserver/displayserver.h"
#include "util/apple/cf_helpers.h"
#include "util/edidHelper.h"

#include <CoreGraphics/CoreGraphics.h>

// DDC/CI
#ifdef __aarch64__
typedef CFTypeRef IOAVServiceRef;
extern IOAVServiceRef IOAVServiceCreate(CFAllocatorRef allocator) __attribute__((weak_import));
extern IOAVServiceRef IOAVServiceCreateWithService(CFAllocatorRef allocator, io_service_t service) __attribute__((weak_import));
extern IOReturn IOAVServiceCopyEDID(IOAVServiceRef service, CFDataRef* x2) __attribute__((weak_import));
extern IOReturn IOAVServiceReadI2C(IOAVServiceRef service, uint32_t chipAddress, uint32_t offset, void* outputBuffer, uint32_t outputBufferSize) __attribute__((weak_import));
extern IOReturn IOAVServiceWriteI2C(IOAVServiceRef service, uint32_t chipAddress, uint32_t dataAddress, void* inputBuffer, uint32_t inputBufferSize) __attribute__((weak_import));
#else
// DDC/CI (Intel)
#include <IOKit/IOKitLib.h>
#include <IOKit/graphics/IOGraphicsLib.h>
#include <IOKit/i2c/IOI2CInterface.h>
extern void CGSServiceForDisplayNumber(CGDirectDisplayID display, io_service_t* service) __attribute__((weak_import));
#endif

// ACPI
extern int DisplayServicesGetBrightness(CGDirectDisplayID display, float *brightness) __attribute__((weak_import));

// Works for internal display
static const char* detectWithDisplayServices(const FFDisplayServerResult* displayServer, FFlist* result)
{
    if(DisplayServicesGetBrightness == NULL)
        return "DisplayServices function DisplayServicesGetBrightness is not available";

    FF_LIST_FOR_EACH(FFDisplayResult, display, displayServer->displays)
    {
        if (display->type == FF_DISPLAY_TYPE_BUILTIN || display->type == FF_DISPLAY_TYPE_UNKNOWN)
        {
            float value;
            if(DisplayServicesGetBrightness((CGDirectDisplayID) display->id, &value) == kCGErrorSuccess)
            {
                FFBrightnessResult* brightness = (FFBrightnessResult*) ffListAdd(result);
                brightness->current = value;
                brightness->max = 1;
                brightness->min = 0;
                ffStrbufInitCopy(&brightness->name, &display->name);
            }
        }
    }

    return NULL;
}

#ifdef __aarch64__
// https://github.com/waydabber/m1ddc
// Works for Apple Silicon and USB-C adapter connection ( but not HTMI )
static const char* detectWithDdcci(FF_MAYBE_UNUSED const FFDisplayServerResult* displayServer, FFBrightnessOptions* options, FFlist* result)
{
    if (!IOAVServiceCreate || !IOAVServiceReadI2C)
        return "IOAVService is not available";

    FF_IOOBJECT_AUTO_RELEASE io_iterator_t iterator = IO_OBJECT_NULL;
    if (IOServiceGetMatchingServices(MACH_PORT_NULL, IOServiceMatching("DCPAVServiceProxy"), &iterator) != kIOReturnSuccess)
        return "IOServiceGetMatchingServices() failed";

    io_registry_entry_t registryEntry;
    while ((registryEntry = IOIteratorNext(iterator)) != IO_OBJECT_NULL)
    {
        FF_CFTYPE_AUTO_RELEASE IOAVServiceRef service = NULL;
        {
            FF_IOOBJECT_AUTO_RELEASE io_registry_entry_t entryAv = registryEntry;

            FF_CFTYPE_AUTO_RELEASE CFBooleanRef IOAVServiceUserInterfaceSupported = IORegistryEntryCreateCFProperty(entryAv, CFSTR("IOAVServiceUserInterfaceSupported"), kCFAllocatorDefault, kNilOptions);
            if (IOAVServiceUserInterfaceSupported && !CFBooleanGetValue(IOAVServiceUserInterfaceSupported))
            {
                // IOAVServiceCreateWithService won't work
                continue;
            }

            FF_CFTYPE_AUTO_RELEASE CFStringRef location = IORegistryEntryCreateCFProperty(entryAv, CFSTR("Location"), kCFAllocatorDefault, kNilOptions);
            if (location && CFStringCompare(location, CFSTR("Embedded"), 0) == 0)
            {
                // Builtin display should be handled by DisplayServices
                continue;
            }

            service = IOAVServiceCreateWithService(kCFAllocatorDefault, (io_service_t) registryEntry);
            if (!service) continue;
        }

        {
            uint8_t i2cIn[4] = { 0x82, 0x01, 0x10 /* luminance */ };
            i2cIn[3] = 0x6e ^ i2cIn[0] ^ i2cIn[1] ^ i2cIn[2];

            for (uint32_t i = 0; i < 2; ++i)
            {
                IOAVServiceWriteI2C(service, 0x37, 0x51, i2cIn, ARRAY_SIZE(i2cIn));
                usleep(options->ddcciSleep * 1000);
            }
        }

        uint8_t i2cOut[12] = {};
        if (IOAVServiceReadI2C(service, 0x37, 0x51, i2cOut, ARRAY_SIZE(i2cOut)) == KERN_SUCCESS)
        {
            if (i2cOut[2] != 0x02 || i2cOut[3] != 0x00)
                continue;

            uint32_t current = ((uint32_t) i2cOut[8] << 8u) + (uint32_t) i2cOut[9];
            uint32_t max = ((uint32_t) i2cOut[6] << 8u) + (uint32_t) i2cOut[7];

            FFBrightnessResult* brightness = (FFBrightnessResult*) ffListAdd(result);
            brightness->max = max;
            brightness->min = 0;
            brightness->current = current;
            ffStrbufInit(&brightness->name);

            uint8_t edid[128] = {};
            if (IOAVServiceReadI2C(service, 0x50, 0x00, edid, ARRAY_SIZE(edid)) == KERN_SUCCESS)
                ffEdidGetName(edid, &brightness->name);
        }
    }

    return NULL;
}
#else
static const char* detectWithDdcci(const FFDisplayServerResult* displayServer, FFBrightnessOptions* options, FFlist* result)
{
    if (!CGSServiceForDisplayNumber) return "CGSServiceForDisplayNumber is not available";

    FF_LIST_FOR_EACH(FFDisplayResult, display, displayServer->displays)
    {
        if (display->type == FF_DISPLAY_TYPE_EXTERNAL)
        {
            io_service_t framebuffer = 0;
            CGSServiceForDisplayNumber((CGDirectDisplayID)display->id, &framebuffer);
            if (framebuffer == 0) continue;

            IOItemCount count;
            if (IOFBGetI2CInterfaceCount(framebuffer, &count) != KERN_SUCCESS || count == 0) continue;

            io_service_t interface = 0;
            if (IOFBCopyI2CInterfaceForBus(framebuffer, 0, &interface) != KERN_SUCCESS) continue;

            uint8_t i2cOut[12] = {};
            IOI2CConnectRef connect;
            if (IOI2CInterfaceOpen(interface, kNilOptions, &connect) != KERN_SUCCESS)
            {
                IOObjectRelease(interface);
                continue;
            }

            uint8_t i2cIn[] = { 0x51, 0x82, 0x01, 0x10 /* luminance */, 0 };
            i2cIn[4] = 0x6E ^ i2cIn[0] ^ i2cIn[1] ^ i2cIn[2] ^ i2cIn[3];

            IOI2CRequest request = {
                .commFlags = kNilOptions,
                .sendAddress = 0x6e,
                .sendTransactionType = kIOI2CSimpleTransactionType,
                .sendBuffer = (vm_address_t) i2cIn,
                .sendBytes = ARRAY_SIZE(i2cIn),
                .minReplyDelay = options->ddcciSleep,
                .replyAddress = 0x6F,
                .replySubAddress = 0x51,
                .replyTransactionType = kIOI2CDDCciReplyTransactionType,
                .replyBytes = ARRAY_SIZE(i2cOut),
                .replyBuffer = (vm_address_t) i2cOut,
            };
            IOReturn ret = IOI2CSendRequest(connect, kNilOptions, &request);
            IOI2CInterfaceClose(connect, kNilOptions);
            IOObjectRelease(interface);

            if (ret  != KERN_SUCCESS || request.result != kIOReturnSuccess) continue;

            if (i2cOut[2] != 0x02 || i2cOut[3] != 0x00) continue;

            uint32_t current = ((uint32_t) i2cOut[8] << 8u) + (uint32_t) i2cOut[9];
            uint32_t max = ((uint32_t) i2cOut[6] << 8u) + (uint32_t) i2cOut[7];

            FFBrightnessResult* brightness = (FFBrightnessResult*) ffListAdd(result);
            brightness->max = max;
            brightness->min = 0;
            brightness->current = current;
            ffStrbufInitCopy(&brightness->name, &display->name);
        }
    }

    return NULL;
}
#endif

const char* ffDetectBrightness(FFBrightnessOptions* options, FFlist* result)
{
    const FFDisplayServerResult* displayServer = ffConnectDisplayServer();

    detectWithDisplayServices(displayServer, result);

    if (displayServer->displays.length > result->length)
        detectWithDdcci(displayServer, options, result);

    return NULL;
}

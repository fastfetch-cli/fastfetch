#include "brightness.h"
#include "detection/displayserver/displayserver.h"
#include "util/apple/cf_helpers.h"

#include <CoreGraphics/CoreGraphics.h>

extern int DisplayServicesGetBrightness(CGDirectDisplayID display, float *brightness) __attribute__((weak_import));

// DDC/CI
typedef CFTypeRef IOAVServiceRef;
extern IOAVServiceRef IOAVServiceCreate(CFAllocatorRef allocator);
extern IOAVServiceRef IOAVServiceCreateWithService(CFAllocatorRef allocator, io_service_t service);
extern IOReturn IOAVServiceCopyEDID(IOAVServiceRef service, CFDataRef* x2);
extern IOReturn IOAVServiceReadI2C(IOAVServiceRef service, uint32_t chipAddress, uint32_t offset, void* outputBuffer, uint32_t outputBufferSize);
extern IOReturn IOAVServiceWriteI2C(IOAVServiceRef service, uint32_t chipAddress, uint32_t dataAddress, void* inputBuffer, uint32_t inputBufferSize);

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

const char* ffDetectBrightness(FFlist* result)
{
    if(DisplayServicesGetBrightness == NULL)
        return "DisplayServices function DisplayServicesGetBrightness is not available";

    const FFDisplayServerResult* displayServer = ffConnectDisplayServer();

    FF_LIST_FOR_EACH(FFDisplayResult, display, displayServer->displays)
    {
        float value;
        if(DisplayServicesGetBrightness((CGDirectDisplayID) display->id, &value) == kCGErrorSuccess)
        {
            FFBrightnessResult* brightness = (FFBrightnessResult*) ffListAdd(result);
            brightness->value = value * 100;
            ffStrbufInitCopy(&brightness->name, &display->name);
            continue;
        }
    }

    if (!instance.config.allowSlowOperations)
        return NULL;

    // https://github.com/waydabber/m1ddc
    // This only works for Apple Silicon and USB-C adapter connection ( but not HTMI )
    FF_CFTYPE_AUTO_RELEASE IOAVServiceRef service = IOAVServiceCreate(kCFAllocatorDefault);
    uint8_t i2cData[12] = { 0x82, 0x01, 0x00 };
    i2cData[3] = 0x6e ^ i2cData[0] ^ i2cData[1] ^ i2cData[2] ^ i2cData[3];

    for (uint32_t i = 0; i < 3; ++i)
    {
        IOAVServiceWriteI2C(service, 0x37, 0x51, i2cData, 4);
        usleep(10000);
    }

    memset(i2cData, 0, sizeof(i2cData));
    if (IOAVServiceReadI2C(service, 0x37, 0x51, i2cData, sizeof(i2cData)) == KERN_SUCCESS)
    {
        uint8_t current = i2cData[9], max = i2cData[7];

        FFBrightnessResult* brightness = (FFBrightnessResult*) ffListAdd(result);
        brightness->value = (float) current * 100.f / max;
        ffStrbufInit(&brightness->name);

        uint8_t edid[128] = {};
        if (IOAVServiceReadI2C(service, 0x50, 0x00, edid, sizeof(edid)) == KERN_SUCCESS)
            getNameFromEdid(edid, &brightness->name);
    }

    return NULL;
}

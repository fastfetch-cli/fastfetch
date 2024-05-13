#include "physicalmemory.h"
#include "common/processing.h"
#include "util/smbiosHelper.h"
#include "util/stringUtils.h"

#import <Foundation/Foundation.h>

static void appendDevice(
    FFlist* result,
    NSString* type,
    NSString* vendor,
    NSString* size,

    // Intel only
    NSString* locator,
    NSString* serial,
    NSString* partNumber,
    NSString* speed,
    bool ecc)
{
    FFPhysicalMemoryResult* device = ffListAdd(result);
    ffStrbufInitS(&device->type, type.UTF8String);
    ffStrbufInit(&device->formFactor);
    ffStrbufInitS(&device->locator, locator.UTF8String);
    ffStrbufInitS(&device->vendor, vendor.UTF8String);
    FFPhysicalMemoryUpdateVendorString(device);
    ffStrbufInitS(&device->serial, serial.UTF8String);
    ffCleanUpSmbiosValue(&device->serial);
    ffStrbufInitS(&device->partNumber, partNumber.UTF8String);
    ffCleanUpSmbiosValue(&device->partNumber);
    device->size = 0;
    device->maxSpeed = 0;
    device->runningSpeed = 0;
    device->ecc = ecc;

    if (size)
    {
        char* unit = NULL;
        device->size = strtoul(size.UTF8String, &unit, 10);
        if (*unit == ' ') ++unit;

        switch (*unit)
        {
            case 'G': device->size *= 1024ULL * 1024 * 1024; break;
            case 'M': device->size *= 1024ULL * 1024; break;
            case 'K': device->size *= 1024ULL; break;
            case 'T': device->size *= 1024ULL * 1024 * 1024 * 1024; break;
        }
    }

    if (speed)
    {
        char* unit = NULL;
        device->maxSpeed = (uint32_t) strtoul(speed.UTF8String, &unit, 10);
        if (*unit == ' ') ++unit;

        switch (*unit)
        {
            case 'T': device->maxSpeed *= 1000 * 1000; break;
            case 'G': device->maxSpeed *= 1000; break;
            case 'K': device->maxSpeed /= 1000; break;
        }
        device->runningSpeed = device->maxSpeed;
    }
}

const char* ffDetectPhysicalMemory(FFlist* result)
{
    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
    if (ffProcessAppendStdOut(&buffer, (char* const[]) {
        "system_profiler",
        "SPMemoryDataType",
        "-xml",
        "-detailLevel",
        "full",
        NULL
    }) != NULL)
        return "Starting `system_profiler SPMemoryDataType -xml -detailLevel full` failed";

    NSArray* arr = [NSPropertyListSerialization propertyListWithData:[NSData dataWithBytes:buffer.chars length:buffer.length]
                    options:NSPropertyListImmutable
                    format:nil
                    error:nil];
    if (!arr || !arr.count)
        return "system_profiler SPMemoryDataType returned an empty array";

    for (NSDictionary* data in arr[0][@"_items"])
    {
        if (data[@"_items"])
        {
            // for Intel
            for (NSDictionary* item in data[@"_items"])
            {
                appendDevice(result,
                    item[@"dimm_type"],
                    item[@"dimm_manufacturer"],
                    item[@"dimm_size"],
                    item[@"_name"],
                    item[@"dimm_serial_number"],
                    item[@"dimm_part_number"],
                    item[@"dimm_speed"],
                    !![data[@"global_ecc_state"] isEqualToString:@"ecc_enabled"]);
            }
        }
        else
        {
            // for Apple Silicon
            appendDevice(result,
                data[@"dimm_type"],
                data[@"dimm_manufacturer"],
                data[@"SPMemoryDataType"],
                nil,
                nil,
                nil,
                nil,
                false);
        }
    }

    return NULL;
}

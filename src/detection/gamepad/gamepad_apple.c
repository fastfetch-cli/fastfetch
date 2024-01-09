#include "gamepad.h"
#include "util/apple/cf_helpers.h"
#include "util/mallocHelper.h"

#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDLib.h>

static void enumSet(IOHIDDeviceRef value, FFlist* results)
{
    FFGamepadDevice* device = (FFGamepadDevice*) ffListAdd(results);
    ffStrbufInit(&device->identifier);
    ffStrbufInit(&device->name);
    device->battery = 0;

    CFStringRef manufacturer = IOHIDDeviceGetProperty(value, CFSTR(kIOHIDManufacturerKey));
    ffCfStrGetString(manufacturer, &device->name);

    CFStringRef product = IOHIDDeviceGetProperty(value, CFSTR(kIOHIDProductKey));
    if (device->name.length)
    {
        ffCfStrGetString(product, &device->identifier);
        ffStrbufAppendC(&device->name, ' ');
        ffStrbufAppend(&device->name, &device->identifier);
    }
    else
    {
        ffCfStrGetString(product, &device->name);
    }

    CFStringRef serialNumber = IOHIDDeviceGetProperty(value, CFSTR(kIOHIDSerialNumberKey));
    ffCfStrGetString(serialNumber, &device->identifier);
}

const char* ffDetectGamepad(FFlist* devices /* List of FFGamepadDevice */)
{
    IOHIDManagerRef FF_CFTYPE_AUTO_RELEASE manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    if (IOHIDManagerOpen(manager, kIOHIDOptionsTypeNone) != kIOReturnSuccess)
        return "IOHIDManagerOpen() failed";

    CFDictionaryRef FF_CFTYPE_AUTO_RELEASE matching1 = CFDictionaryCreate(kCFAllocatorDefault, (const void **)(CFStringRef[]){
        CFSTR(kIOHIDDeviceUsagePageKey),
        CFSTR(kIOHIDDeviceUsageKey)
    }, (const void **)(CFNumberRef[]){
        ffCfCreateInt(kHIDPage_GenericDesktop),
        ffCfCreateInt(kHIDUsage_GD_Joystick)
    }, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFDictionaryRef FF_CFTYPE_AUTO_RELEASE matching2 = CFDictionaryCreate(kCFAllocatorDefault, (const void **)(CFStringRef[]){
        CFSTR(kIOHIDDeviceUsagePageKey),
        CFSTR(kIOHIDDeviceUsageKey)
    }, (const void **)(CFNumberRef[]){
        ffCfCreateInt(kHIDPage_GenericDesktop),
        ffCfCreateInt(kHIDUsage_GD_GamePad)
    }, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFArrayRef FF_CFTYPE_AUTO_RELEASE matchings = CFArrayCreate(kCFAllocatorDefault, (const void **)(CFTypeRef[]){
        matching1, matching2
    }, 2, &kCFTypeArrayCallBacks);
    IOHIDManagerSetDeviceMatchingMultiple(manager, matchings);

    CFSetRef FF_CFTYPE_AUTO_RELEASE set = IOHIDManagerCopyDevices(manager);
    if (set)
        CFSetApplyFunction(set, (CFSetApplierFunction) &enumSet, devices);
    IOHIDManagerClose(manager, kIOHIDOptionsTypeNone);

    return NULL;
}

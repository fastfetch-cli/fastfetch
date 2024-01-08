#include "gamepad.h"
#include "util/apple/cf_helpers.h"

#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDLib.h>

static void enumSet(IOHIDDeviceRef value, FFlist* results)
{
    FFGamepadDevice* device = (FFGamepadDevice*) ffListAdd(results);
    ffStrbufInit(&device->identifier);
    ffStrbufInit(&device->name);
    device->battery = 0;

    CFStringRef serialNumber = IOHIDDeviceGetProperty(value, CFSTR(kIOHIDSerialNumberKey));
    ffCfStrGetString(serialNumber, &device->identifier);

    CFStringRef product = IOHIDDeviceGetProperty(value, CFSTR(kIOHIDProductKey));
    ffCfStrGetString(product, &device->name);
}

const char* ffDetectGamepad(FFlist* devices /* List of FFGamepadDevice */)
{
    IOHIDManagerRef FF_CFTYPE_AUTO_RELEASE manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    if (IOHIDManagerOpen(manager, kIOHIDOptionsTypeNone) != kIOReturnSuccess)
        return "IOHIDManagerOpen() failed";

    CFNumberRef FF_CFTYPE_AUTO_RELEASE genericDesktop = ffCfCreateInt(kHIDPage_GenericDesktop);
    CFNumberRef FF_CFTYPE_AUTO_RELEASE gamePad = ffCfCreateInt(kHIDUsage_GD_GamePad);
    CFDictionaryRef FF_CFTYPE_AUTO_RELEASE matching = CFDictionaryCreate(kCFAllocatorDefault, (const void **)(CFStringRef[]){
        CFSTR(kIOHIDDeviceUsagePageKey),
        CFSTR(kIOHIDDeviceUsageKey)
    }, (const void **)(CFNumberRef[]){
        genericDesktop,
        gamePad
    }, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    IOHIDManagerSetDeviceMatching(manager, matching);

    CFSetRef FF_CFTYPE_AUTO_RELEASE set = IOHIDManagerCopyDevices(manager);
    if (set)
        CFSetApplyFunction(set, (CFSetApplierFunction) &enumSet, devices);
    IOHIDManagerClose(manager, 0);

    return NULL;
}

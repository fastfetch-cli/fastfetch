#include "gamepad.h"
#include "util/apple/cf_helpers.h"

#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDLib.h>

static void enumSet(const void* value, void *context)
{
    CFStringRef product = IOHIDDeviceGetProperty((IOHIDDeviceRef) value, CFSTR(kIOHIDProductKey));
    CFStringRef serialNumber = IOHIDDeviceGetProperty((IOHIDDeviceRef) value, CFSTR(kIOHIDSerialNumberKey));
    FFGamepadDevice* device = (FFGamepadDevice*) ffListAdd((FFlist*) context);
    ffStrbufInit(&device->identifier);
    ffCfStrGetString(serialNumber, &device->identifier);
    ffStrbufInit(&device->name);
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
        CFSetApplyFunction(set, &enumSet, devices);
    IOHIDManagerClose(manager, 0);

    return NULL;
}

#include "mouse.h"
#include "util/apple/cf_helpers.h"
#include "util/mallocHelper.h"

#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDLib.h>

static void enumSet(IOHIDDeviceRef value, FFlist* results)
{
    FFMouseDevice* device = (FFMouseDevice*) ffListAdd(results);
    ffStrbufInit(&device->serial);
    ffStrbufInit(&device->name);

    CFStringRef product = IOHIDDeviceGetProperty(value, CFSTR(kIOHIDProductKey));
    ffCfStrGetString(product, &device->name);

    CFStringRef serialNumber = IOHIDDeviceGetProperty(value, CFSTR(kIOHIDSerialNumberKey));
    ffCfStrGetString(serialNumber, &device->serial);
}

const char* ffDetectMouse(FFlist* devices /* List of FFMouseDevice */)
{
    IOHIDManagerRef FF_CFTYPE_AUTO_RELEASE manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    if (IOHIDManagerOpen(manager, kIOHIDOptionsTypeNone) != kIOReturnSuccess)
        return "IOHIDManagerOpen() failed";

    CFDictionaryRef FF_CFTYPE_AUTO_RELEASE matching1 = CFDictionaryCreate(kCFAllocatorDefault, (const void **)(CFStringRef[]){
        CFSTR(kIOHIDDeviceUsagePageKey),
        CFSTR(kIOHIDDeviceUsageKey)
    }, (const void **)(CFNumberRef[]){
        ffCfCreateInt(kHIDPage_GenericDesktop),
        ffCfCreateInt(kHIDUsage_GD_Mouse)
    }, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    IOHIDManagerSetDeviceMatching(manager, matching1);

    CFSetRef FF_CFTYPE_AUTO_RELEASE set = IOHIDManagerCopyDevices(manager);
    if (set)
        CFSetApplyFunction(set, (CFSetApplierFunction) &enumSet, devices);
    IOHIDManagerClose(manager, kIOHIDOptionsTypeNone);

    return NULL;
}

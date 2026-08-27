#include "gamepad.h"
#include "common/apple/cf_helpers.h"
#include "common/mallocHelper.h"

#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDLib.h>

static void enumSet(IOHIDDeviceRef value, FFlist* results) {
    FFGamepadDevice* device = FF_LIST_ADD(FFGamepadDevice, *results);
    ffStrbufInit(&device->serial);
    ffStrbufInit(&device->name);
    device->battery = 0;

    CFStringRef manufacturer = IOHIDDeviceGetProperty(value, CFSTR(kIOHIDManufacturerKey));
    ffCfStrGetString(manufacturer, &device->name);

    CFStringRef product = IOHIDDeviceGetProperty(value, CFSTR(kIOHIDProductKey));
    if (device->name.length) {
        ffCfStrGetString(product, &device->serial);
        ffStrbufAppendC(&device->name, ' ');
        ffStrbufAppend(&device->name, &device->serial);
    } else {
        ffCfStrGetString(product, &device->name);
    }

    CFStringRef serialNumber = IOHIDDeviceGetProperty(value, CFSTR(kIOHIDSerialNumberKey));
    ffCfStrGetString(serialNumber, &device->serial);
}

const char* ffDetectGamepad(FFlist* devices /* List of FFGamepadDevice */) {
    FF_CFTYPE_AUTO_RELEASE IOHIDManagerRef manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    if (IOHIDManagerOpen(manager, kIOHIDOptionsTypeNone) != kIOReturnSuccess) {
        return "IOHIDManagerOpen() failed";
    }

    FF_CFTYPE_AUTO_RELEASE CFDictionaryRef matching1 = CFDictionaryCreate(kCFAllocatorDefault, (const void**) (CFStringRef[]) { CFSTR(kIOHIDDeviceUsagePageKey), CFSTR(kIOHIDDeviceUsageKey) }, (const void**) (CFNumberRef[]) { ffCfCreateInt(kHIDPage_GenericDesktop), ffCfCreateInt(kHIDUsage_GD_Joystick) }, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    FF_CFTYPE_AUTO_RELEASE CFDictionaryRef matching2 = CFDictionaryCreate(kCFAllocatorDefault, (const void**) (CFStringRef[]) { CFSTR(kIOHIDDeviceUsagePageKey), CFSTR(kIOHIDDeviceUsageKey) }, (const void**) (CFNumberRef[]) { ffCfCreateInt(kHIDPage_GenericDesktop), ffCfCreateInt(kHIDUsage_GD_GamePad) }, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    FF_CFTYPE_AUTO_RELEASE CFArrayRef matchings = CFArrayCreate(kCFAllocatorDefault, (const void**) (CFTypeRef[]) { matching1, matching2 }, 2, &kCFTypeArrayCallBacks);
    IOHIDManagerSetDeviceMatchingMultiple(manager, matchings);

    FF_CFTYPE_AUTO_RELEASE CFSetRef set = IOHIDManagerCopyDevices(manager);
    if (set) {
        CFSetApplyFunction(set, (CFSetApplierFunction) &enumSet, devices);
    }
    IOHIDManagerClose(manager, kIOHIDOptionsTypeNone);

    return nullptr;
}

#include "monitor.h"
#include "detection/displayserver/displayserver.h"
#include "util/apple/cf_helpers.h"
#include "util/edidHelper.h"

#import <AppKit/NSScreen.h>
#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>

#ifdef MAC_OS_X_VERSION_10_15
extern CFDictionaryRef CoreDisplay_DisplayCreateInfoDictionary(CGDirectDisplayID display) __attribute__((weak_import));
#else
#include <IOKit/graphics/IOGraphicsLib.h>
#endif

static bool detectHdrSupportWithNSScreen(FFDisplayResult* display)
{
    NSScreen* mainScreen = NSScreen.mainScreen;
    if (display->primary)
    {
        #ifdef MAC_OS_X_VERSION_10_15
        return mainScreen.maximumPotentialExtendedDynamicRangeColorComponentValue > 1;
        #else
        return mainScreen.maximumExtendedDynamicRangeColorComponentValue > 1;
        #endif
    }
    else
    {
        for (NSScreen* screen in NSScreen.screens)
        {
            if (screen == mainScreen) continue;
            NSNumber* screenNumber = [screen.deviceDescription valueForKey:@"NSScreenNumber"];
            if (screenNumber && screenNumber.longValue == (long) display->id)
            {
                #ifdef MAC_OS_X_VERSION_10_15
                return screen.maximumPotentialExtendedDynamicRangeColorComponentValue > 1;
                #else
                return screen.maximumExtendedDynamicRangeColorComponentValue > 1;
                #endif
            }
        }
    }
    return false;
}

const char* ffDetectMonitor(FFlist* results)
{
    #ifdef MAC_OS_X_VERSION_10_15
    if(!CoreDisplay_DisplayCreateInfoDictionary) return "CoreDisplay_DisplayCreateInfoDictionary is not available";
    #endif
    const FFDisplayServerResult* displayServer = ffConnectDisplayServer();

    FF_LIST_FOR_EACH(FFDisplayResult, display, displayServer->displays)
    {
        #ifdef MAC_OS_X_VERSION_10_15
        CFDictionaryRef FF_CFTYPE_AUTO_RELEASE displayInfo = CoreDisplay_DisplayCreateInfoDictionary((CGDirectDisplayID) display->id);
        #else
        io_service_t servicePort = CGDisplayIOServicePort((CGDirectDisplayID) display->id);
        CFDictionaryRef FF_CFTYPE_AUTO_RELEASE displayInfo = IODisplayCreateInfoDictionary(servicePort, kIODisplayOnlyPreferredName);
        #endif
        if(!displayInfo) continue;

        bool isVirtual = false;
        if (ffCfDictGetBool(displayInfo, CFSTR("kCGDisplayIsVirtualDevice"), &isVirtual) == NULL && isVirtual)
            continue;

        uint8_t edidData[128] = {};
        uint32_t edidLength = 0;
        if (ffCfDictGetData(displayInfo, CFSTR(kIODisplayEDIDKey), 0, sizeof(edidData), edidData, &edidLength) == NULL)
        {
            uint32_t width, height;
            ffEdidGetPhysicalResolution(edidData, &width, &height);
            if (width > 0 && height > 0)
            {
                FFMonitorResult* monitor = (FFMonitorResult*) ffListAdd(results);
                ffStrbufInitCopy(&monitor->name, &display->name);
                monitor->width = width;
                monitor->height = height;
                ffEdidGetPhysicalSize(edidData, &monitor->physicalWidth, &monitor->physicalHeight);
                monitor->hdrCompatible = CFDictionaryContainsKey(displayInfo, CFSTR("ReferencePeakHDRLuminance")) ||
                    detectHdrSupportWithNSScreen(display);
                continue;
            }
        }

        int width, height;
        if (ffCfDictGetInt(displayInfo, CFSTR("kCGDisplayPixelWidth"), &width) || // Default resolution (limited by connectors, GPUs, etc.)
            ffCfDictGetInt(displayInfo, CFSTR("kCGDisplayPixelHeight"), &height) ||
            width <= 0 || height <= 0)
            continue;

        FFMonitorResult* monitor = (FFMonitorResult*) ffListAdd(results);
        monitor->width = (uint32_t) width;
        monitor->height = (uint32_t) height;
        ffStrbufInitCopy(&monitor->name, &display->name);

        CGSize size = CGDisplayScreenSize((CGDirectDisplayID) display->id);
        monitor->physicalWidth = (uint32_t) (size.width + 0.5);
        monitor->physicalHeight = (uint32_t) (size.height + 0.5);
        monitor->hdrCompatible = CFDictionaryContainsKey(displayInfo, CFSTR("ReferencePeakHDRLuminance")) ||
            detectHdrSupportWithNSScreen(display);
        monitor->serial = CGDisplaySerialNumber((CGDirectDisplayID) display->id);

        FF_CFTYPE_AUTO_RELEASE CFArrayRef modes = CGDisplayCopyAllDisplayModes((CGDirectDisplayID) display->id, NULL);
        double maxRefreshRate = 0;
        for (uint32_t j = 0; j < CFArrayGetCount(modes); ++j)
        {
            CGDisplayModeRef mode = (CGDisplayModeRef) CFArrayGetValueAtIndex(modes, j);
            if (CGDisplayModeGetWidth(mode) == (uint32_t) width && CGDisplayModeGetHeight(mode) == (uint32_t) height)
            {
                double refreshRate = CGDisplayModeGetRefreshRate(mode);
                if (refreshRate > maxRefreshRate) maxRefreshRate = refreshRate;
            }
        }
        monitor->refreshRate = maxRefreshRate;

        int64_t year, week;
        if (ffCfDictGetInt64(displayInfo, CFSTR("DisplayYearManufacture"), &year) == NULL)
            monitor->manufactureYear = (uint16_t) year;
        if (ffCfDictGetInt64(displayInfo, CFSTR("DisplayWeekManufacture"), &week) == NULL)
            monitor->manufactureWeek = (uint16_t) week;
    }

    return NULL;
}

#include "monitor.h"
#include "detection/displayserver/displayserver.h"
#include "util/apple/cf_helpers.h"
#include "util/edidHelper.h"

#import <AppKit/NSScreen.h>
#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>

extern CFDictionaryRef CoreDisplay_DisplayCreateInfoDictionary(CGDirectDisplayID display) __attribute__((weak_import));

static bool detectHdrSupportWithNSScreen(FFDisplayResult* display)
{
    NSScreen* mainScreen = NSScreen.mainScreen;
    if (display->primary)
        return mainScreen.maximumPotentialExtendedDynamicRangeColorComponentValue > 1;
    else
    {
        for (NSScreen* screen in NSScreen.screens)
        {
            if (screen == mainScreen) continue;
            NSNumber* screenNumber = [screen.deviceDescription valueForKey:@"NSScreenNumber"];
            if (screenNumber && screenNumber.longValue == (long) display->id)
            {
                return screen.maximumPotentialExtendedDynamicRangeColorComponentValue > 1;
            }
        }
    }
    return false;
}

const char* ffDetectMonitor(FFlist* results)
{
    if(!CoreDisplay_DisplayCreateInfoDictionary) return "CoreDisplay_DisplayCreateInfoDictionary is not available";

    const FFDisplayServerResult* displayServer = ffConnectDisplayServer();

    FF_LIST_FOR_EACH(FFDisplayResult, display, displayServer->displays)
    {
        CFDictionaryRef FF_CFTYPE_AUTO_RELEASE displayInfo = CoreDisplay_DisplayCreateInfoDictionary((CGDirectDisplayID) display->id);
        if(!displayInfo) continue;

        uint8_t edidData[128] = {};
        uint32_t edidLength = 0;
        if (false || !ffCfDictGetData(displayInfo, CFSTR("IODisplayEDID"), 0, sizeof(edidData), edidData, &edidLength))
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
    }

    return NULL;
}

#include "displayserver.h"
#include "util/apple/cf_helpers.h"
#include "util/stringUtils.h"
#include "util/edidHelper.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <CoreGraphics/CGDirectDisplay.h>
#include <CoreVideo/CVDisplayLink.h>

extern Boolean CoreDisplay_Display_SupportsHDRMode(CGDirectDisplayID display) __attribute__((weak_import));
extern Boolean CoreDisplay_Display_IsHDRModeEnabled(CGDirectDisplayID display) __attribute__((weak_import));
#ifdef MAC_OS_X_VERSION_10_15
extern CFDictionaryRef CoreDisplay_DisplayCreateInfoDictionary(CGDirectDisplayID display) __attribute__((weak_import));
#else
#include <IOKit/graphics/IOGraphicsLib.h>
#endif

static void detectDisplays(FFDisplayServerResult* ds)
{
    CGDirectDisplayID screens[128];
    uint32_t screenCount;
    if(CGGetOnlineDisplayList(sizeof(screens) / sizeof(screens[0]), screens, &screenCount) != kCGErrorSuccess)
        return;

    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
    for(uint32_t i = 0; i < screenCount; i++)
    {
        CGDirectDisplayID screen = screens[i];
        CGDisplayModeRef mode = CGDisplayCopyDisplayMode(screen);
        if(mode)
        {
            //https://github.com/glfw/glfw/commit/aab08712dd8142b642e2042e7b7ba563acd07a45
            double refreshRate = CGDisplayModeGetRefreshRate(mode);

            if (refreshRate == 0)
            {
                CVDisplayLinkRef link;
                if(CVDisplayLinkCreateWithCGDisplay(screen, &link) == kCVReturnSuccess)
                {
                    const CVTime time = CVDisplayLinkGetNominalOutputVideoRefreshPeriod(link);
                    if (!(time.flags & kCVTimeIsIndefinite))
                        refreshRate = time.timeScale / (double) time.timeValue; //59.97...
                    CVDisplayLinkRelease(link);
                }
            }

            ffStrbufClear(&buffer);
            CFDictionaryRef FF_CFTYPE_AUTO_RELEASE displayInfo = NULL;
            #ifdef MAC_OS_X_VERSION_10_15
            if(CoreDisplay_DisplayCreateInfoDictionary)
                displayInfo = CoreDisplay_DisplayCreateInfoDictionary(screen);
            #else
            {
                io_service_t servicePort = CGDisplayIOServicePort(screen);
                displayInfo = IODisplayCreateInfoDictionary(servicePort, kIODisplayOnlyPreferredName);
            }
            #endif
            uint32_t physicalWidth = 0, physicalHeight = 0;
            if(displayInfo)
            {
                CFDictionaryRef productNames;
                if(!ffCfDictGetDict(displayInfo, CFSTR(kDisplayProductName), &productNames))
                    ffCfDictGetString(productNames, CFSTR("en_US"), &buffer);

                // CGDisplayScreenSize reports invalid result for external displays on old Intel MacBook Pro
                CFDataRef edidRef = (CFDataRef) CFDictionaryGetValue(displayInfo, CFSTR(kIODisplayEDIDKey));
                if (edidRef && CFGetTypeID(edidRef) == CFDataGetTypeID())
                {
                    const uint8_t* edidData = CFDataGetBytePtr(edidRef);
                    uint32_t edidLength = (uint32_t) CFDataGetLength(edidRef);
                    if (edidLength >= 128)
                        ffEdidGetPhysicalSize(edidData, &physicalWidth, &physicalHeight);
                }
            }

            if (!physicalWidth || !physicalHeight)
            {
                CGSize size = CGDisplayScreenSize(screen);
                physicalWidth = (uint32_t) (size.width + 0.5);
                physicalHeight = (uint32_t) (size.height + 0.5);
            }

            FFDisplayResult* display = ffdsAppendDisplay(ds,
                (uint32_t)CGDisplayModeGetPixelWidth(mode),
                (uint32_t)CGDisplayModeGetPixelHeight(mode),
                refreshRate,
                (uint32_t)CGDisplayModeGetWidth(mode),
                (uint32_t)CGDisplayModeGetHeight(mode),
                (uint32_t)CGDisplayRotation(screen),
                &buffer,
                CGDisplayIsBuiltin(screen) ? FF_DISPLAY_TYPE_BUILTIN : FF_DISPLAY_TYPE_EXTERNAL,
                CGDisplayIsMain(screen),
                (uint64_t)screen,
                physicalWidth,
                physicalHeight
            );
            if (display)
            {
                // https://stackoverflow.com/a/33519316/9976392
                // Also shitty, but better than parsing `CFCopyDescription(mode)`
                CFDictionaryRef dict = (CFDictionaryRef) *((int64_t *)mode + 2);
                if (CFGetTypeID(dict) == CFDictionaryGetTypeID())
                {
                    int32_t bitDepth;
                    ffCfDictGetInt(dict, kCGDisplayBitsPerSample, &bitDepth);
                    display->bitDepth = (uint8_t) bitDepth;
                }

                if (display->type == FF_DISPLAY_TYPE_BUILTIN)
                    display->hdrStatus = CFDictionaryContainsKey(displayInfo, CFSTR("ReferencePeakHDRLuminance"))
                        ? FF_DISPLAY_HDR_STATUS_SUPPORTED : FF_DISPLAY_HDR_STATUS_UNSUPPORTED;
                else if (CoreDisplay_Display_SupportsHDRMode)
                {
                    if (CoreDisplay_Display_SupportsHDRMode(screen))
                    {
                        display->hdrStatus = FF_DISPLAY_HDR_STATUS_SUPPORTED;
                        if (CoreDisplay_Display_IsHDRModeEnabled && CoreDisplay_Display_IsHDRModeEnabled(screen))
                            display->hdrStatus = FF_DISPLAY_HDR_STATUS_ENABLED;
                    }
                    else
                        display->hdrStatus = FF_DISPLAY_HDR_STATUS_UNSUPPORTED;
                }

                display->serial = CGDisplaySerialNumber(screen);
                int value;
                if (ffCfDictGetInt(displayInfo, CFSTR(kDisplayYearOfManufacture), &value) == NULL)
                    display->manufactureYear = (uint16_t) value;
                if (ffCfDictGetInt(displayInfo, CFSTR(kDisplayWeekOfManufacture), &value) == NULL)
                    display->manufactureWeek = (uint16_t) value;
            }
            CGDisplayModeRelease(mode);
        }
        CGDisplayRelease(screen);
    }
}

void ffConnectDisplayServerImpl(FFDisplayServerResult* ds)
{
    {
        FF_CFTYPE_AUTO_RELEASE CFMachPortRef port = CGWindowServerCreateServerPort();
        if (port)
        {
            ffStrbufSetStatic(&ds->wmProcessName, "WindowServer");
            ffStrbufSetStatic(&ds->wmPrettyName, "Quartz Compositor");
        }
    }
    ffStrbufSetStatic(&ds->dePrettyName, "Aqua");

    detectDisplays(ds);
}

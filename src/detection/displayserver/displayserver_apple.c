#include "displayserver.h"
#include "util/apple/cf_helpers.h"
#include "util/stringUtils.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <CoreGraphics/CGDirectDisplay.h>
#include <CoreVideo/CVDisplayLink.h>

#ifdef MAC_OS_X_VERSION_10_15
extern CFDictionaryRef CoreDisplay_DisplayCreateInfoDictionary(CGDirectDisplayID display) __attribute__((weak_import));
extern Boolean CoreDisplay_Display_IsHDRModeEnabled(CGDirectDisplayID display) __attribute__((weak_import));
#else
#include <IOKit/graphics/IOGraphicsLib.h>
#endif

static void detectDisplays(FFDisplayServerResult* ds)
{
    CGDirectDisplayID screens[128];
    uint32_t screenCount;
    if(CGGetOnlineDisplayList(sizeof(screens) / sizeof(screens[0]), screens, &screenCount) != kCGErrorSuccess)
        return;

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

            FF_STRBUF_AUTO_DESTROY name = ffStrbufCreate();
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
            if(displayInfo)
            {
                CFDictionaryRef productNames;
                if(!ffCfDictGetDict(displayInfo, CFSTR(kDisplayProductName), &productNames))
                    ffCfDictGetString(productNames, CFSTR("en_US"), &name);
            }

            CGSize size = CGDisplayScreenSize(screen);

            FFDisplayResult* display = ffdsAppendDisplay(ds,
                (uint32_t)CGDisplayModeGetPixelWidth(mode),
                (uint32_t)CGDisplayModeGetPixelHeight(mode),
                refreshRate,
                (uint32_t)CGDisplayModeGetWidth(mode),
                (uint32_t)CGDisplayModeGetHeight(mode),
                (uint32_t)CGDisplayRotation(screen),
                &name,
                CGDisplayIsBuiltin(screen) ? FF_DISPLAY_TYPE_BUILTIN : FF_DISPLAY_TYPE_EXTERNAL,
                CGDisplayIsMain(screen),
                (uint64_t)screen,
                (uint32_t) (size.width + 0.5),
                (uint32_t) (size.height + 0.5)
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

                #ifdef MAC_OS_X_VERSION_10_15
                if (CoreDisplay_Display_IsHDRModeEnabled)
                {
                    display->hdrEnabled = CoreDisplay_Display_IsHDRModeEnabled(screen);
                }
                #endif
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

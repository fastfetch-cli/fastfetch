#include "displayserver.h"
#include "util/apple/cf_helpers.h"
#include "util/stringUtils.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <CoreGraphics/CGDirectDisplay.h>
#include <CoreVideo/CVDisplayLink.h>

extern CFDictionaryRef CoreDisplay_DisplayCreateInfoDictionary(CGDirectDisplayID display) __attribute__((weak_import));
#ifndef MAC_OS_X_VERSION_10_15
#import <IOKit/graphics/IOGraphicsLib.h>
extern CFDictionaryRef CoreDisplay_IODisplayCreateInfoDictionary(io_service_t framebuffer, IOOptionBits options)  __attribute__((weak_import));
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
                        refreshRate = time.timeScale / (double) time.timeValue + 0.5; //59.97...
                    CVDisplayLinkRelease(link);
                }
            }

            FF_STRBUF_AUTO_DESTROY name = ffStrbufCreate();
            #ifdef MAC_OS_X_VERSION_10_15
            if(CoreDisplay_DisplayCreateInfoDictionary)
            {
                CFDictionaryRef FF_CFTYPE_AUTO_RELEASE displayInfo = CoreDisplay_DisplayCreateInfoDictionary(screen);
                if(displayInfo)
                {
                    CFDictionaryRef productNames;
                    if(!ffCfDictGetDict(displayInfo, CFSTR(kDisplayProductName), &productNames))
                        ffCfDictGetString(productNames, CFSTR("en_US"), &name);
                }
            }
            #else
            if(CoreDisplay_IODisplayCreateInfoDictionary)
            {
                io_service_t servicePort = CGDisplayIOServicePort(screen);
                CFDictionaryRef FF_CFTYPE_AUTO_RELEASE displayInfo = CoreDisplay_IODisplayCreateInfoDictionary(servicePort, kIODisplayOnlyPreferredName);
                if(displayInfo)
                {
                    CFDictionaryRef productNames;
                    if(!ffCfDictGetDict(displayInfo, CFSTR(kDisplayProductName), &productNames))
                        ffCfDictGetString(productNames, CFSTR("en_US"), &name);
                }
            }
            #endif

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
                // Shitty code
                uint8_t bitDepth = 0;
                FF_CFTYPE_AUTO_RELEASE CFStringRef desc = CFCopyDescription(mode);
                CFRange start = CFStringFind(desc, CFSTR("BitsPerSample = "), 0);
                if (start.location != kCFNotFound)
                {
                    for (CFIndex idx = start.location + start.length; idx < CFStringGetLength(desc); ++idx)
                    {
                        UniChar ch = CFStringGetCharacterAtIndex(desc, idx);
                        if (!ffCharIsDigit((char) ch))
                            break;
                        bitDepth = (uint8_t) (bitDepth * 10 + (ch - '0'));
                    }
                }
                display->bitDepth = bitDepth;
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

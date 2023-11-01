#include "displayserver.h"
#include "util/apple/cf_helpers.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <CoreGraphics/CGDirectDisplay.h>
#include <CoreVideo/CVDisplayLink.h>

extern CFDictionaryRef CoreDisplay_DisplayCreateInfoDictionary(CGDirectDisplayID display) __attribute__((weak_import));

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

            ffdsAppendDisplay(ds,
                (uint32_t)CGDisplayModeGetPixelWidth(mode),
                (uint32_t)CGDisplayModeGetPixelHeight(mode),
                refreshRate,
                (uint32_t)CGDisplayModeGetWidth(mode),
                (uint32_t)CGDisplayModeGetHeight(mode),
                (uint32_t)CGDisplayRotation(screen),
                &name,
                CGDisplayIsBuiltin(screen) ? FF_DISPLAY_TYPE_BUILTIN : FF_DISPLAY_TYPE_EXTERNAL,
                CGDisplayIsMain(screen),
                (uint64_t)screen
            );
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
            ffStrbufInitStatic(&ds->wmProcessName, "WindowServer");
            ffStrbufInitStatic(&ds->wmPrettyName, "Quartz Compositor");
        }
        else
        {
            ffStrbufInit(&ds->wmProcessName);
            ffStrbufInit(&ds->wmPrettyName);
        }
    }
    ffStrbufInit(&ds->wmProtocolName);

    ffStrbufInit(&ds->deProcessName);
    ffStrbufInitStatic(&ds->dePrettyName, "Aqua");

    ffListInitA(&ds->displays, sizeof(FFDisplayResult), 4);
    detectDisplays(ds);
}

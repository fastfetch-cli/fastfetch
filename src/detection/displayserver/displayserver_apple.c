#include "displayserver.h"
#include "common/sysctl.h"
#include "util/apple/cf_helpers.h"
#include "util/mallocHelper.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <CoreGraphics/CGDirectDisplay.h>
#include <CoreVideo/CVDisplayLink.h>

extern CFDictionaryRef CoreDisplay_DisplayCreateInfoDictionary(CGDirectDisplayID display) __attribute__((weak_import));

static void detectDisplays(FFDisplayServerResult* ds, bool detectName)
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

            FF_STRBUF_AUTO_DESTROY name;
            ffStrbufInit(&name);
            if(detectName && CoreDisplay_DisplayCreateInfoDictionary)
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
                &name,
                CGDisplayIsBuiltin(screen) ? FF_DISPLAY_TYPE_BUILTIN : FF_DISPLAY_TYPE_EXTERNAL
            );
            CGDisplayModeRelease(mode);
        }
        CGDisplayRelease(screen);
    }
}

static void detectWMPlugin(FFstrbuf* name)
{
    int request[] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL};
    u_int requestLength = sizeof(request) / sizeof(*request);

    size_t length = 0;
    FF_AUTO_FREE struct kinfo_proc* processes = ffSysctlGetData(request, requestLength, &length);
    if(processes == NULL)
        return;

    for(size_t i = 0; i < length / sizeof(struct kinfo_proc); i++)
    {
        const char* comm = processes[i].kp_proc.p_comm;

        if(
            strcasecmp(comm, "spectacle") != 0 &&
            strcasecmp(comm, "amethyst") != 0 &&
            strcasecmp(comm, "kwm") != 0 &&
            strcasecmp(comm, "chunkwm") != 0 &&
            strcasecmp(comm, "yabai") != 0 &&
            strcasecmp(comm, "rectangle") != 0
        ) continue;

        ffStrbufAppendS(name, comm);
        name->chars[0] = (char) toupper(name->chars[0]);
        break;
    }
}

void ffConnectDisplayServerImpl(FFDisplayServerResult* ds, const FFinstance* instance)
{
    FF_UNUSED(instance);

    ffStrbufInitS(&ds->wmProcessName, "WindowServer");
    ffStrbufInitS(&ds->wmPrettyName, "Quartz Compositor");
    ffStrbufInit(&ds->wmProtocolName);

    if(instance->config.allowSlowOperations)
    {
        FF_STRBUF_AUTO_DESTROY name;
        ffStrbufInit(&name);
        detectWMPlugin(&name);
        if(name.length)
            ffStrbufAppendF(&ds->wmPrettyName, " (with %s)", name.chars);
    }

    ffStrbufInit(&ds->deProcessName);
    ffStrbufInit(&ds->dePrettyName);
    ffStrbufInit(&ds->deVersion);
    ffStrbufInit(&ds->deProcessName);
    ffStrbufAppendS(&ds->dePrettyName, "Aqua");

    ffListInitA(&ds->displays, sizeof(FFDisplayResult), 4);
    detectDisplays(ds, instance->config.displayDetectName);
}

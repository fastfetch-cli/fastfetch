#include "displayserver.h"
#include "common/sysctl.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <CoreGraphics/CGDirectDisplay.h>

static void detectResolution(FFDisplayServerResult* ds)
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
            ffdsAppendResolution(ds,
                (uint32_t)CGDisplayModeGetWidth(mode),
                (uint32_t)CGDisplayModeGetHeight(mode),
                (uint32_t)CGDisplayModeGetRefreshRate(mode)
            );
            CGDisplayModeRelease(mode);
        }
        CGDisplayRelease(screen);
    }
}

static void detectWM(FFDisplayServerResult* ds)
{
    int request[] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL};
    u_int requestLength = sizeof(request) / sizeof(*request);

    size_t length = 0;
    struct kinfo_proc* processes = ffSysctlGetData(request, requestLength, &length);
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

        ffStrbufAppendS(&ds->wmProcessName, comm);
        ffStrbufAppendS(&ds->wmPrettyName, comm);
        ds->wmPrettyName.chars[0] = (char) toupper(ds->wmPrettyName.chars[0]);
        break;
    }

    free(processes);
}

void ffConnectDisplayServerImpl(FFDisplayServerResult* ds, const FFinstance* instance)
{
    FF_UNUSED(instance);

    ffStrbufInit(&ds->wmProcessName);
    ffStrbufInit(&ds->wmPrettyName);
    ffStrbufInitA(&ds->wmProtocolName, 0);
    detectWM(ds);
    if(ds->wmProcessName.length == 0)
    {
        ffStrbufAppendS(&ds->wmProcessName, "quartz");
        ffStrbufAppendS(&ds->wmPrettyName, "Quartz Compositor");
    }

    ffStrbufInit(&ds->deProcessName);
    ffStrbufInit(&ds->dePrettyName);
    ffStrbufInitA(&ds->deVersion, 0);
    ffStrbufAppendS(&ds->deProcessName, "aqua");
    ffStrbufAppendS(&ds->dePrettyName, "Aqua");

    ffListInitA(&ds->resolutions, sizeof(FFResolutionResult), 4);
    detectResolution(ds);
}

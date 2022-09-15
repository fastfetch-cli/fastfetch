#include "displayserver.h"
#include "common/library.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/sysctl.h>
#include <ApplicationServices/ApplicationServices.h>

//Resolution code heavily inspired by displayplacer <3

typedef union
{
    uint8_t rawData[0xDC];
    struct
    {
        uint32_t mode;
        uint32_t flags;		// 0x4
        uint32_t width;		// 0x8
        uint32_t height;	// 0xC
        uint32_t depth;		// 0x10
        uint32_t dc2[42];
        uint16_t dc3;
        uint16_t freq;		// 0xBC
        uint32_t dc4[4];
        float density;		// 0xD0
    } derived;
} modes_D4;

void CGSGetCurrentDisplayMode(CGDirectDisplayID display, int* modeNum);
void CGSGetDisplayModeDescriptionOfLength(CGDirectDisplayID display, int idx, modes_D4* mode, int length);

static void detectResolution(FFDisplayServerResult* ds)
{
    CGDisplayCount screenCount;
    CGGetOnlineDisplayList(INT_MAX, NULL, &screenCount);
    if(screenCount == 0)
        return;

    CGDirectDisplayID* screens = malloc(screenCount * sizeof(CGDirectDisplayID));
    CGGetOnlineDisplayList(INT_MAX, screens, &screenCount);

    for(uint32_t i = 0; i < screenCount; i++)
    {
        int modeID;
        CGSGetCurrentDisplayMode(screens[i], &modeID);
        modes_D4 mode;
        CGSGetDisplayModeDescriptionOfLength(screens[i], modeID, &mode, 0xD4);

        uint32_t refreshRate = ffdsParseRefreshRate(mode.derived.freq);
        ffdsAppendResolution(ds, mode.derived.width, mode.derived.height, refreshRate);
    }

    free(screens);
}

static void detectWM(FFDisplayServerResult* ds)
{
    int request[] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL};
    u_int requestLength = sizeof(request) / sizeof(*request);

    size_t length = 0;
    if(sysctl(request, requestLength, NULL, &length, NULL, 0) != 0)
        return;

    struct kinfo_proc* processes = malloc(length);
    if(processes == NULL)
        return;

    if(sysctl(request, requestLength, processes, &length, NULL, 0) != 0)
    {
        free(processes);
        return;
    }

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

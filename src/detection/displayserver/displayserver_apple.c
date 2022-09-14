#include "displayserver.h"
#include "common/library.h"

#include <stdlib.h>
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
    {
        dlclose(cg);
        return;
    }

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

void ffConnectDisplayServerImpl(FFDisplayServerResult* ds, const FFinstance* instance)
{
    FF_UNUSED(instance);

    ffStrbufInit(&ds->wmProcessName);
    ffStrbufAppendS(&ds->wmProcessName, "quartz");

    ffStrbufInit(&ds->wmPrettyName);
    ffStrbufAppendS(&ds->wmPrettyName, "Quartz Compositor");

    ffStrbufInit(&ds->deProcessName);
    ffStrbufAppendS(&ds->deProcessName, "aqua");

    ffStrbufInit(&ds->dePrettyName);
    ffStrbufAppendS(&ds->dePrettyName, "Aqua");

    ffListInitA(&ds->resolutions, sizeof(FFResolutionResult), 4);
    detectResolution(ds);

    ffStrbufInitA(&ds->deVersion, 0);
    ffStrbufInitA(&ds->wmProtocolName, 0);
}

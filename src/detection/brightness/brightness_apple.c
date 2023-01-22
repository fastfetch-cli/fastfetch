#include "brightness.h"

#include <CoreGraphics/CoreGraphics.h>

extern int DisplayServicesGetBrightness(CGDirectDisplayID display, float *brightness) __attribute__((weak_import));

const char* ffDetectBrightness(FFlist* result)
{
    if(DisplayServicesGetBrightness == NULL)
        return "DisplayServices function DisplayServicesGetBrightness is not available";

    CGDirectDisplayID screens[128];
    uint32_t screenCount;
    if(CGGetOnlineDisplayList(sizeof(screens) / sizeof(screens[0]), screens, &screenCount) != kCGErrorSuccess)
        return "CGGetOnlineDisplayList() failed";

    for(uint32_t i = 0; i < screenCount; i++)
    {
        CGDirectDisplayID screen = screens[i];
        float brightness;
        if(DisplayServicesGetBrightness(screen, &brightness) != kCGErrorSuccess)
            continue;

        FFBrightnessResult* display = (FFBrightnessResult*) ffListAdd(result);
        if(CGDisplayIsBuiltin(screen))
            ffStrbufInitS(&display->name, "built-in");
        else
            ffStrbufInitF(&display->name, "external-%u", (unsigned) screen);
        display->value = brightness * 100;
    }

    return NULL;
}

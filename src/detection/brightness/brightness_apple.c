#include "brightness.h"
#include "util/apple/cf_helpers.h"

#include <CoreGraphics/CoreGraphics.h>

extern int DisplayServicesGetBrightness(CGDirectDisplayID display, float *brightness) __attribute__((weak_import));
extern CFDictionaryRef CoreDisplay_DisplayCreateInfoDictionary(CGDirectDisplayID display) __attribute__((weak_import));

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
        display->value = brightness * 100;
        ffStrbufInit(&display->name);

        if(CoreDisplay_DisplayCreateInfoDictionary)
        {
            CFDictionaryRef FF_CFTYPE_AUTO_RELEASE displayInfo = CoreDisplay_DisplayCreateInfoDictionary(screen);
            if(displayInfo)
            {
                CFDictionaryRef productNames;
                if(!ffCfDictGetDict(displayInfo, CFSTR(kDisplayProductName), &productNames))
                {
                    if(!ffCfDictGetString(productNames, CFSTR("en_US"), &display->name))
                        continue;
                }
            }
        }

        if(CGDisplayIsBuiltin(screen))
            ffStrbufAppendS(&display->name, "built-in");
        else
            ffStrbufAppendF(&display->name, "external-%u", (unsigned) screen);
    }

    return NULL;
}

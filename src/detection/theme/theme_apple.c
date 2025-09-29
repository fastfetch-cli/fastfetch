#include "theme.h"

#include "detection/os/os.h"

const char* ffDetectTheme(FFThemeResult* result)
{
    const FFOSResult* os = ffDetectOS();

    char* str_end;
    const char* version = os->version.chars;
    unsigned long osNum = strtoul(version, &str_end, 10);
    if (str_end != version)
    {
        if (osNum > 15) { // Tahoe
            ffStrbufSetStatic(&result->theme1, "Liquid Glass");
        } else if (osNum < 10) {
            ffStrbufSetStatic(&result->theme1, "Platinum");
        } else {
            ffStrbufSetStatic(&result->theme1, "Aqua");
        }
    }

    return NULL;
}

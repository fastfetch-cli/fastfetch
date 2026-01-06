#include "brightness.h"

#include "common/sysctl.h"

const char* ffDetectBrightness(FF_MAYBE_UNUSED FFBrightnessOptions* options, FFlist* result)
{
    // https://man.netbsd.org/NetBSD-10.1/acpiout.4#DESCRIPTION
    char key[] = "hw.acpi.acpiout0.brightness";
    char* pn = key + strlen("hw.acpi.acpiout");

    for (uint32_t i = 0; i <= 9; ++i)
    {
        *pn = (char) ('0' + i);
        int value = ffSysctlGetInt(key, -1);
        if (value == -1) continue;

        FFBrightnessResult* brightness = (FFBrightnessResult*) ffListAdd(result);
        ffStrbufInitF(&brightness->name, "acpiout%d", i);

        brightness->max = 100;
        brightness->min = 0;
        brightness->current = value;
        brightness->builtin = true;
    }
    return NULL;
}

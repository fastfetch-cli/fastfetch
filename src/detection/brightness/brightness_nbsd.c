#include "brightness.h"

#include "common/sysctl.h"

const char* ffDetectBrightness(FF_MAYBE_UNUSED FFBrightnessOptions* options, FFlist* result)
{
    // https://man.netbsd.org/NetBSD-10.1/acpiout.4#DESCRIPTION

    int value = ffSysctlGetInt("hw.acpi.acpiout0.brightness", -1);
    if (value == -1)
        return "sysctl(hw.acpi.acpiout0.brightness) failed";

    FFBrightnessResult* brightness = (FFBrightnessResult*) ffListAdd(result);
    ffStrbufInitStatic(&brightness->name, "acpiout");

    brightness->max = 100;
    brightness->min = 0;
    brightness->current = value;
    return NULL;
}

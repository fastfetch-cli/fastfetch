#include "fastfetch.h"
#include "battery.h"

const char* ffDetectBatteryImpl(FFinstance* instance, FFlist* results)
{
    FF_UNUSED(instance, results)
    return "Not supported on this platform";
}

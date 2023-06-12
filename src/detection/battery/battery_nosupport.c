#include "fastfetch.h"
#include "battery.h"

const char* ffDetectBattery(FFinstance* instance, FFBatteryOptions* options, FFlist* results)
{
    FF_UNUSED(instance, options, results)
    return "Not supported on this platform";
}

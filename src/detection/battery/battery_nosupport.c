#include "fastfetch.h"
#include "battery.h"

const char* ffDetectBattery(FFBatteryOptions* options, FFlist* results)
{
    FF_UNUSED(options, results)
    return "Not supported on this platform";
}

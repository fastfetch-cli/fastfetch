#include "wifi.h"

void ffDetectWifi(const FFinstance* instance, FFWifiResult* result)
{
    FF_UNUSED(instance);
    ffStrbufAppendS(&result->error, "Not supported on this platform");
}

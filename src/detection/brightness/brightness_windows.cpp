extern "C"
{
#include "brightness.h"
}
#include "util/windows/wmi.hpp"

extern "C"
const char* ffDetectBrightness(FFlist* result)
{
    FFWmiQuery query(L"SELECT CurrentBrightness, InstanceName FROM WmiMonitorBrightness WHERE Active = true", nullptr, FFWmiNamespace::WMI);
    if(!query)
        return "Query WMI service failed";

    while(FFWmiRecord record = query.next())
    {
        FFBrightnessResult* display = (FFBrightnessResult*) ffListAdd(result);
        ffStrbufInit(&display->name);
        record.getString(L"InstanceName", &display->name);
        ffStrbufSubstrAfterFirstC(&display->name, '\\');
        ffStrbufSubstrBeforeFirstC(&display->name, '\\');

        uint64_t brightness;
        record.getUnsigned(L"CurrentBrightness", &brightness);
        display->value = (float) brightness;
    }
    return NULL;
}

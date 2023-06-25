extern "C"
{
#include "brightness.h"
}
#include "util/windows/wmi.hpp"
#include "util/windows/unicode.hpp"

extern "C"
const char* ffDetectBrightness(FFlist* result)
{
    FFWmiQuery query(L"SELECT CurrentBrightness, InstanceName FROM WmiMonitorBrightness WHERE Active = true", nullptr, FFWmiNamespace::WMI);
    if(!query)
        return "Query WMI service failed";

    while(FFWmiRecord record = query.next())
    {
        if(FFWmiVariant vtValue = record.get(L"CurrentBrightness"))
        {
            FFBrightnessResult* display = (FFBrightnessResult*) ffListAdd(result);
            display->value = vtValue.get<uint8_t>();

            ffStrbufInit(&display->name);
            if (FFWmiVariant vtName = record.get(L"InstanceName"))
            {
                ffStrbufSetWSV(&display->name, vtName.get<std::wstring_view>());
                ffStrbufSubstrAfterFirstC(&display->name, '\\');
                ffStrbufSubstrBeforeFirstC(&display->name, '\\');
            }
        }
    }
    return NULL;
}

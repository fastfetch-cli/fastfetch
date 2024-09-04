extern "C"
{
#include "brightness.h"
#include "detection/displayserver/displayserver.h"
#include "common/library.h"
}
#include "util/windows/wmi.hpp"
#include "util/windows/unicode.hpp"

#include <highlevelmonitorconfigurationapi.h>
#include <physicalmonitorenumerationapi.h>

static const char* detectWithWmi(FFlist* result)
{
    FFWmiQuery query(L"SELECT CurrentBrightness, InstanceName FROM WmiMonitorBrightness WHERE Active = true", nullptr, FFWmiNamespace::WMI);
    if(!query)
        return "Query WMI service failed";

    while(FFWmiRecord record = query.next())
    {
        if(FFWmiVariant vtValue = record.get(L"CurrentBrightness"))
        {
            FFBrightnessResult* brightness = (FFBrightnessResult*) ffListAdd(result);
            brightness->max = 100;
            brightness->min = 0;
            brightness->current = vtValue.get<uint8_t>();

            ffStrbufInit(&brightness->name);
            if (FFWmiVariant vtName = record.get(L"InstanceName"))
            {
                ffStrbufSetWSV(&brightness->name, vtName.get<std::wstring_view>());
                ffStrbufSubstrAfterFirstC(&brightness->name, '\\');
                ffStrbufSubstrBeforeFirstC(&brightness->name, '\\');
            }
        }
    }
    return NULL;
}

static const char* detectWithDdcci(const FFDisplayServerResult* displayServer, FFlist* result)
{
    FF_LIBRARY_LOAD(dxva2, "dlopen dxva2" FF_LIBRARY_EXTENSION " failed", "dxva2" FF_LIBRARY_EXTENSION, 1)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(dxva2, GetPhysicalMonitorsFromHMONITOR)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(dxva2, GetMonitorBrightness)

    FF_LIST_FOR_EACH(FFDisplayResult, display, displayServer->displays)
    {
        PHYSICAL_MONITOR physicalMonitor;
        if (ffGetPhysicalMonitorsFromHMONITOR((HMONITOR)(uintptr_t) display->id, 1, &physicalMonitor))
        {
            DWORD min = 0, curr = 0, max = 0;
            if (ffGetMonitorBrightness(physicalMonitor.hPhysicalMonitor, &min, &curr, &max))
            {
                FFBrightnessResult* brightness = (FFBrightnessResult*) ffListAdd(result);

                if (display->name.length)
                    ffStrbufInitCopy(&brightness->name, &display->name);
                else
                    ffStrbufInitWS(&brightness->name, physicalMonitor.szPhysicalMonitorDescription);

                brightness->max = max;
                brightness->min = min;
                brightness->current = curr;
            }
        }
    }
    return NULL;
}

static bool hasBuiltinDisplay(const FFDisplayServerResult* displayServer)
{
    FF_LIST_FOR_EACH(FFDisplayResult, display, displayServer->displays)
    {
        if (display->type == FF_DISPLAY_TYPE_BUILTIN || display->type == FF_DISPLAY_TYPE_UNKNOWN)
            return true;
    }
    return false;
}

extern "C"
const char* ffDetectBrightness(FF_MAYBE_UNUSED FFBrightnessOptions* options, FFlist* result)
{
    const FFDisplayServerResult* displayServer = ffConnectDisplayServer();

    if (hasBuiltinDisplay(displayServer))
        detectWithWmi(result);

    if (result->length < displayServer->displays.length)
        detectWithDdcci(displayServer, result);
    return NULL;
}

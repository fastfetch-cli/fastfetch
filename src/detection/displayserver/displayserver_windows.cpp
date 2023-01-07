extern "C" {
#include "displayserver.h"
#include "detection/os/os.h"
}
#include "util/windows/wmi.hpp"

#include <dwmapi.h>
#include <winuser.h>

extern "C"
void ffConnectDisplayServerImpl(FFDisplayServerResult* ds, const FFinstance* instance)
{
    FF_UNUSED(instance);

    BOOL enabled;
    if(SUCCEEDED(DwmIsCompositionEnabled(&enabled)) && enabled == TRUE)
    {
        ffStrbufInitS(&ds->wmProcessName, "dwm.exe");
        ffStrbufInitS(&ds->wmPrettyName, "Desktop Window Manager");
    }
    else
    {
        ffStrbufInitS(&ds->wmProcessName, "internal");
        ffStrbufInitS(&ds->wmPrettyName, "internal");
    }
    ffStrbufInit(&ds->wmProtocolName);
    ffStrbufInit(&ds->deProcessName);
    ffStrbufInit(&ds->dePrettyName);
    ffStrbufInit(&ds->deVersion);
    ffListInit(&ds->resolutions, sizeof(FFResolutionResult));

    DISPLAY_DEVICEW displayDevice;
    displayDevice.cb = sizeof(DISPLAY_DEVICEW);
    for(DWORD devNum = 0; EnumDisplayDevicesW(NULL, devNum, &displayDevice, 0) != 0; ++devNum)
    {
        if(!(displayDevice.StateFlags & DISPLAY_DEVICE_ACTIVE))
            continue;
        DEVMODEW devMode;
        devMode.dmSize = sizeof(DEVMODEW);
        if(EnumDisplaySettingsW(displayDevice.DeviceName, ENUM_CURRENT_SETTINGS, &devMode) == 0)
            continue;

        ffdsAppendResolution(ds, devMode.dmPelsWidth, devMode.dmPelsHeight, devMode.dmDisplayFrequency, -1);
    }

    //TODO: support multiple monitors
    if(ds->resolutions.length == 1)
    {
        FFWmiQuery query(L"SELECT CurrentBrightness FROM WmiMonitorBrightness WHERE Active = true", nullptr, FFWmiNamespace::WMI);
        if(FFWmiRecord record = query.next())
        {
            uint64_t brightness;
            record.getUnsigned(L"CurrentBrightness", &brightness);
            FF_LIST_FOR_EACH(FFResolutionResult, resolution, ds->resolutions)
            {
                resolution->brightness = (int) brightness;
            }
        }
    }

    //https://github.com/hykilpikonna/hyfetch/blob/master/neofetch#L2067
    const FFOSResult* os = ffDetectOS(instance);
    if(
        ffStrbufEqualS(&os->version, "11") ||
        ffStrbufEqualS(&os->version, "10") ||
        ffStrbufEqualS(&os->version, "2022") ||
        ffStrbufEqualS(&os->version, "2019") ||
        ffStrbufEqualS(&os->version, "2016")
    ) ffStrbufSetS(&ds->dePrettyName, "Fluent");
    else if(
        ffStrbufEqualS(&os->version, "8") ||
        ffStrbufEqualS(&os->version, "81.") ||
        ffStrbufEqualS(&os->version, "2012 R2") ||
        ffStrbufEqualS(&os->version, "2012")
    ) ffStrbufSetS(&ds->dePrettyName, "Metro");
    else
        ffStrbufSetS(&ds->dePrettyName, "Aero");
}

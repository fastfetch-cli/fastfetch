#include "displayserver.h"
#include "detection/os/os.h"

#include <dwmapi.h>
#include <WinUser.h>

typedef struct
{
    WCHAR* deviceName;
    uint32_t width;
    uint32_t height;
} DataBundle;

static WINBOOL enumMonitorProc(HMONITOR hMonitor, FF_MAYBE_UNUSED HDC hDC, FF_MAYBE_UNUSED LPRECT rc, LPARAM lparam)
{
    MONITORINFOEXW mi = { .cbSize = sizeof(mi) };
    DataBundle* data = (DataBundle*) lparam;
    if(GetMonitorInfoW(hMonitor, (MONITORINFO *)&mi) && wcscmp(mi.szDevice, data->deviceName) == 0)
    {
        data->width = (uint32_t) mi.rcMonitor.right;
        data->height = (uint32_t) mi.rcMonitor.bottom;
        return FALSE;
    }
    return TRUE;
}

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
    ffListInit(&ds->displays, sizeof(FFDisplayResult));

    DISPLAY_DEVICEW displayDevice = { .cb = sizeof(DISPLAY_DEVICEW) };
    for(DWORD devNum = 0; EnumDisplayDevicesW(NULL, devNum, &displayDevice, 0) != 0; ++devNum)
    {
        if(!(displayDevice.StateFlags & DISPLAY_DEVICE_ACTIVE))
            continue;
        DEVMODEW devMode = { .dmSize = sizeof(DEVMODEW) };
        if(EnumDisplaySettingsW(displayDevice.DeviceName, ENUM_CURRENT_SETTINGS, &devMode) == 0)
            continue;

        DataBundle data = {
            .deviceName = displayDevice.DeviceName,
            .width = 0,
            .height = 0,
        };
        EnumDisplayMonitors(NULL, NULL, enumMonitorProc, (LPARAM)&data);
        ffdsAppendDisplay(ds, devMode.dmPelsWidth, devMode.dmPelsHeight, devMode.dmDisplayFrequency, data.width, data.height);
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

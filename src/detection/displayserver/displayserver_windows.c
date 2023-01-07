#include "displayserver.h"
#include "detection/os/os.h"
#include "util/mallocHelper.h"

#include <dwmapi.h>
#include <WinUser.h>
#include <wchar.h>
#include <highlevelmonitorconfigurationapi.h>

static WINBOOL enumMonitorProc(HMONITOR hMonitor, FF_UNUSED_PARAM HDC hDC, FF_UNUSED_PARAM LPRECT rc, LPARAM data)
{
    MONITORINFOEXW mi = { .cbSize = sizeof(mi) };
    DISPLAY_DEVICEW* displayDevice = (DISPLAY_DEVICEW *) data;
    if(GetMonitorInfoW(hMonitor, (MONITORINFO *)&mi) && wcscmp(mi.szDevice, displayDevice->DeviceName) == 0)
    {
        DWORD arraySize;
        if(GetNumberOfPhysicalMonitorsFromHMONITOR(hMonitor, &arraySize) && arraySize > 0)
        {
            PHYSICAL_MONITOR* FF_AUTO_FREE physicalMonitorArray = malloc(arraySize * sizeof(*physicalMonitorArray));
            if(GetPhysicalMonitorsFromHMONITOR(hMonitor, arraySize, physicalMonitorArray))
            {
                DWORD minValue, currentValue, maxValue;
                if(GetMonitorBrightness(physicalMonitorArray[0].hPhysicalMonitor, &minValue, &currentValue, &maxValue))
                    displayDevice->StateFlags = currentValue * 100 / maxValue;

                DestroyPhysicalMonitors(arraySize, physicalMonitorArray);
            }
        }
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
    ffListInit(&ds->resolutions, sizeof(FFResolutionResult));

    DISPLAY_DEVICEW displayDevice = { .cb = sizeof(DISPLAY_DEVICEW) };
    for(DWORD devNum = 0; EnumDisplayDevicesW(NULL, devNum, &displayDevice, 0) != 0; ++devNum)
    {
        if(!(displayDevice.StateFlags & DISPLAY_DEVICE_ACTIVE))
            continue;
        DEVMODEW devMode = { .dmSize = sizeof(DEVMODEW) };
        if(EnumDisplaySettingsW(displayDevice.DeviceName, ENUM_CURRENT_SETTINGS, &devMode) == 0)
            continue;

        displayDevice.StateFlags = (DWORD) -1;
        EnumDisplayMonitors(NULL, NULL, enumMonitorProc, (LPARAM)(void*) &displayDevice);

        ffdsAppendResolution(ds, devMode.dmPelsWidth, devMode.dmPelsHeight, devMode.dmDisplayFrequency, (int) displayDevice.StateFlags);
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

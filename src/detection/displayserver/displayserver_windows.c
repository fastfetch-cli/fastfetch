#include "displayserver.h"

#include <dwmapi.h>
#include <WinUser.h>

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

        FFResolutionResult* resolution = (FFResolutionResult*) ffListAdd(&ds->resolutions);
        resolution->width = devMode.dmPelsWidth;
        resolution->height = devMode.dmPelsHeight;
        resolution->refreshRate = devMode.dmDisplayFrequency;
    }
}

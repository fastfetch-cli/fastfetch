#include "displayserver.h"
#include "detection/os/os.h"

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

        ffdsAppendResolution(ds, devMode.dmPelsWidth, devMode.dmPelsHeight, devMode.dmDisplayFrequency);
    }

    //https://github.com/hykilpikonna/hyfetch/blob/master/neofetch#L2067
    const FFOSResult* os = ffDetectOS(instance);
    if(ffStrbufCompS(&os->version, "11") == 0 || ffStrbufCompS(&os->version, "10") == 0)
        ffStrbufSetS(&ds->dePrettyName, "Fluent");
    else if(ffStrbufCompS(&os->version, "8") == 0 || ffStrbufStartsWithS(&os->version, "8."))
        ffStrbufSetS(&ds->dePrettyName, "Metro");
    else
        ffStrbufSetS(&ds->dePrettyName, "Aero");
}

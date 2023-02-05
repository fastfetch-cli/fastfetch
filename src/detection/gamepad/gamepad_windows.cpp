extern "C" {
    #include "gamepad.h"
}

#include <initguid.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

CALLBACK BOOL DIEnumDevicesCallback(LPCDIDEVICEINSTANCEA lpddi, LPVOID pvRef)
{
    FFlist* devices = (FFlist*) pvRef;

    FFGamepadDevice* device = (FFGamepadDevice*) ffListAdd(devices);
    ffStrbufInitF(&device->identifier, "{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
        lpddi->guidInstance.Data1, lpddi->guidInstance.Data2, lpddi->guidInstance.Data3,
        lpddi->guidInstance.Data4[0], lpddi->guidInstance.Data4[1], lpddi->guidInstance.Data4[2], lpddi->guidInstance.Data4[3],
        lpddi->guidInstance.Data4[4], lpddi->guidInstance.Data4[5], lpddi->guidInstance.Data4[6], lpddi->guidInstance.Data4[7]);
    ffStrbufInitS(&device->name, lpddi->tszInstanceName);

    return DIENUM_CONTINUE;
}

extern "C"
const char* ffDetectGamepad(FF_MAYBE_UNUSED const FFinstance* instance, FFlist* devices /* List of FFGamepadDevice */)
{
    IDirectInput8A* dinput;
    if (!SUCCEEDED(DirectInput8Create(GetModuleHandleW(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8A, (void**)&dinput, NULL)))
        return "DirectInput8Create() failed";
    dinput->EnumDevices(DI8DEVCLASS_GAMECTRL, DIEnumDevicesCallback, devices, DIEDFL_ATTACHEDONLY);
    dinput->Release();
    return NULL;
}

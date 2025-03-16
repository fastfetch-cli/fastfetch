extern "C" {
#include "gamepad.h"
}
#include <Joystick.h>

const char* ffDetectGamepad(FFlist* devices /* List of FFGamepadDevice */)
{
    BJoystick js;
    for (int32 i = 0, n = js.CountDevices(); i < n; ++i)
    {
        char name[B_OS_NAME_LENGTH];
        if (js.GetDeviceName(i, name) == B_OK)
        {
            FFGamepadDevice* device = (FFGamepadDevice*) ffListAdd(devices);
            ffStrbufInit(&device->serial);
            ffStrbufInitS(&device->name, name);
            device->battery = 0;
        }
    }
    return NULL;
}

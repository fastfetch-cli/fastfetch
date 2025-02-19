extern "C" {
#include "keyboard.h"
}

#include <interface/Input.h>
#include <support/List.h>

const char* ffDetectKeyboard(FFlist* devices /* List of FFKeyboardDevice */)
{
    BList list;

    if (get_input_devices(&list) != B_OK)
        return "get_input_devices() failed";

    for (int32 i = 0, n = list.CountItems(); i < n; i++)
    {
        BInputDevice *device = (BInputDevice *) list.ItemAt(i);
        if (device->Type() != B_KEYBOARD_DEVICE || !device->IsRunning())
            continue;

        FFKeyboardDevice* item = (FFKeyboardDevice*) ffListAdd(devices);
        ffStrbufInit(&item->serial);
        ffStrbufInitS(&item->name, device->Name());
    }

    return NULL;
}

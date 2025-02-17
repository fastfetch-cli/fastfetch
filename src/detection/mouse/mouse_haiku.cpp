extern "C" {
#include "mouse.h"
}

#include <interface/Input.h>
#include <support/List.h>

const char* ffDetectMouse(FFlist* devices /* List of FFMouseDevice */)
{
    BList list;

    if (get_input_devices(&list) != B_OK)
        return "get_input_devices() failed";

    for (int32 i = 0, n = list.CountItems(); i < n; i++)
    {
        BInputDevice *device = (BInputDevice *) list.ItemAt(i);
        if (device->Type() != B_POINTING_DEVICE || !device->IsRunning())
            continue;

        FFMouseDevice* item = (FFMouseDevice*) ffListAdd(devices);
        ffStrbufInit(&item->serial);
        ffStrbufInitS(&item->name, device->Name());
    }

    return NULL;
}

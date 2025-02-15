extern "C" {
#include "mouse.h"
#include "common/io/io.h"
}

#include <interface/Input.h>
#include <support/List.h>

const char* ffDetectMouse(FFlist* devices /* List of FFMouseDevice */)
{
    BList list;
    BInputDevice *device;

    if (get_input_devices(&list) != B_OK)
    {
        return "get_input_devices() failed";
    }

    int32 i, n = list.CountItems();
    for (i = 0; i < n; i++)
    {
        device = (BInputDevice *) list.ItemAt(i);
        if (device->Type() != B_POINTING_DEVICE)
            continue;

        FF_STRBUF_AUTO_DESTROY name = ffStrbufCreateS(device->Name());
        if (!device->IsRunning())
            ffStrbufAppendS(&name, " (stopped)");

        FFMouseDevice* device = (FFMouseDevice*) ffListAdd(devices);
        ffStrbufInit(&device->serial);
        ffStrbufInitMove(&device->name, &name);
    }

    return NULL;
}

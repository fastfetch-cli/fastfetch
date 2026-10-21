extern "C" {
#include "mouse.h"
#include "common/debug.h"
}

#include <interface/Input.h>
#include <support/List.h>

const char* ffDetectMouse(FFlist* devices /* List of FFMouseDevice */) {
    BList list;

    status_t inputDevicesStatus = get_input_devices(&list);
    if (inputDevicesStatus != B_OK) {
        FF_DEBUG("get_input_devices() failed: status_t 0x%08x", (uint32_t) inputDevicesStatus);
        return "get_input_devices() failed";
    }

    for (int32 i = 0, n = list.CountItems(); i < n; i++) {
        BInputDevice* device = (BInputDevice*) list.ItemAt(i);
        if (device->Type() != B_POINTING_DEVICE || !device->IsRunning()) {
            continue;
        }

        FFMouseDevice* item = FF_LIST_ADD(FFMouseDevice, *devices);
        ffStrbufInit(&item->serial);
        ffStrbufInitS(&item->name, device->Name());
    }

    return nullptr;
}

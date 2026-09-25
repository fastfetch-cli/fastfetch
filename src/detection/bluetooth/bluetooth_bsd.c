#include "bluetooth.h"

#define L2CAP_SOCKET_CHECKED
#include <bluetooth.h>

static int enumDev([[maybe_unused]] int sockfd, struct bt_devinfo const* dev, FFlist* devices) {
    FFBluetoothResult* device = FF_LIST_ADD(FFBluetoothResult, *devices);
    ffStrbufInitS(&device->name,
#if __FreeBSD__
        bt_devremote_name_gen(dev->devname, &dev->bdaddr)
#else
        dev->devname
#endif
    );
    ffStrbufInitS(&device->address, bt_ntoa(&dev->bdaddr, nullptr));
    ffStrbufUpperCase(&device->address);
    ffStrbufInit(&device->type);
    device->deviceType = FF_BLUETOOTH_DEVICE_TYPE_CLASSIC_BIT; // Netgraph only carries the BR/EDR stack
    device->battery = 0;
    device->signalQuality = -DBL_MAX;
    device->connected = true;
    return 0;
}

const char* ffDetectBluetooth(FFBluetoothOptions* options, FFlist* devices /* FFBluetoothResult */) {
    // Netgraph carries the BR/EDR stack and nothing else, so there is no second function to dispatch
    // to here: a Low Energy-only configuration simply has nothing to walk.
    if (!(options->showType & FF_BLUETOOTH_DEVICE_TYPE_CLASSIC_BIT)) {
        return nullptr;
    }

    // struct hostent* ent = bt_gethostent();
    if (bt_devenum((void*) enumDev, devices) < 0) {
        return "bt_devenum() failed";
    }

    return nullptr;
}

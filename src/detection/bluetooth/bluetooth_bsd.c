#include "bluetooth.h"

#define L2CAP_SOCKET_CHECKED
#include <bluetooth.h>

static int enumDev(FF_MAYBE_UNUSED int sockfd, struct bt_devinfo const* dev, FFlist* devices)
{
    FFBluetoothResult* device = ffListAdd(devices);
    ffStrbufInitS(&device->name,
        #if __FreeBSD__
        bt_devremote_name_gen(dev->devname, &dev->bdaddr)
        #else
        dev->devname
        #endif
    );
    ffStrbufInitS(&device->address, bt_ntoa(&dev->bdaddr, NULL));
    ffStrbufUpperCase(&device->address);
    ffStrbufInit(&device->type);
    device->battery = 0;
    device->connected = true;
    return 0;
}

const char* ffDetectBluetooth(FF_MAYBE_UNUSED FFBluetoothOptions* options, FF_MAYBE_UNUSED FFlist* devices /* FFBluetoothResult */)
{
    // struct hostent* ent = bt_gethostent();
    if (bt_devenum((void*) enumDev, devices) < 0)
        return "bt_devenum() failed";

    return NULL;
}

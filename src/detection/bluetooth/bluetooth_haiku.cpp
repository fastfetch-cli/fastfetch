extern "C" {
#include "bluetooth.h"
#include "common/io/io.h"
}

#include <bluetooth/LocalDevice.h>

const char* ffDetectBluetooth(FF_MAYBE_UNUSED FFBluetoothOptions* options, FFlist* devices /* FFBluetoothResult */)
{
    using namespace Bluetooth;
    FF_SUPPRESS_IO();

    LocalDevice* dev = LocalDevice::GetLocalDevice();
    if (!dev) return NULL;

    BString devClass;
    dev->GetDeviceClass().DumpDeviceClass(devClass);

    FFBluetoothResult* device = (FFBluetoothResult*) ffListAdd(devices);
    ffStrbufInitS(&device->name, dev->GetFriendlyName());
    ffStrbufInitS(&device->address, bdaddrUtils::ToString(dev->GetBluetoothAddress()).String());
    ffStrbufInitS(&device->type, devClass.String());
    device->battery = 0;
    device->connected = true;

    // TODO: more devices?

    return NULL;
}

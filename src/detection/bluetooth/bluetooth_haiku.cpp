extern "C" {
#include "bluetooth.h"
#include "common/io.h"
}

#include <bluetooth/LocalDevice.h>

const char* ffDetectBluetooth(FFBluetoothOptions* options, FFlist* devices /* FFBluetoothResult */) {
    // The local adapter's device class is a BR/EDR concept and the adapter is the only device this
    // backend has, so a Low Energy-only configuration leaves the list empty.
    if (!(options->showType & FF_BLUETOOTH_DEVICE_TYPE_CLASSIC_BIT)) {
        return nullptr;
    }

    using namespace Bluetooth;
    FF_SUPPRESS_IO();

    LocalDevice* dev = LocalDevice::GetLocalDevice();
    if (!dev) {
        return nullptr;
    }

    BString devClass;
    dev->GetDeviceClass().DumpDeviceClass(devClass);

    FFBluetoothResult* device = FF_LIST_ADD(FFBluetoothResult, *devices);
    ffStrbufInitS(&device->name, dev->GetFriendlyName());
    ffStrbufInitS(&device->address, bdaddrUtils::ToString(dev->GetBluetoothAddress()).String());
    ffStrbufInitS(&device->type, devClass.String());
    device->deviceType = FF_BLUETOOTH_DEVICE_TYPE_CLASSIC_BIT; // The local adapter's device class is a BR/EDR concept
    device->battery = 0;
    device->signalQuality = -DBL_MAX;
    device->connected = true;

    // TODO: more devices?

    return nullptr;
}

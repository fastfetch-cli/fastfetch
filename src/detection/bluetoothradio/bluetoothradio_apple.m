#include "bluetoothradio.h"

#import <IOBluetooth/IOBluetooth.h>

const char* ffDetectBluetoothRadio(FFlist* devices /* FFBluetoothResult */)
{
    IOBluetoothHostController* ctrl = IOBluetoothHostController.defaultController;
    if(!ctrl)
        return "IOBluetoothHostController.defaultController failed";

    FFBluetoothRadioResult* device = ffListAdd(devices);
    ffStrbufInitS(&device->name, ctrl.nameAsString.UTF8String);
    ffStrbufInitS(&device->address, ctrl.addressAsString.UTF8String);
    device->lmpVersion = 0;
    device->hciVersion = 0;
    device->lmpSubversion = 0;
    device->hciRevision = 0;

    #ifdef __aarch64__
    device->vendor = "Apple";
    #else
    device->vendor = "Unknown"; // Hackintosh?
    #endif

    return NULL;
}

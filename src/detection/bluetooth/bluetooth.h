#pragma once

#include "fastfetch.h"
#include "modules/bluetooth/option.h"

// Which radio a device was seen over. A dual-mode device answers on both, and a platform that only
// walks one stack can only report the bit it knows about, so this is a bitfield rather than a value.
typedef enum FFBluetoothDeviceType: uint8_t {
    FF_BLUETOOTH_DEVICE_TYPE_NONE = 0,
    FF_BLUETOOTH_DEVICE_TYPE_CLASSIC_BIT = 1 << 0, // BR/EDR, the BTHENUM enumerator
    FF_BLUETOOTH_DEVICE_TYPE_LE_BIT = 1 << 1,      // Low Energy, the BTHLE enumerator
} FFBluetoothDeviceType;

typedef struct FFBluetoothResult {
    FFstrbuf name;
    FFstrbuf address;
    FFstrbuf type;
    FFBluetoothDeviceType deviceType;
    uint8_t battery;       // 0-100%
    double signalQuality;  // 0-100%, -DBL_MAX if unknown
    bool connected;
} FFBluetoothResult;

const char* ffDetectBluetooth(FFBluetoothOptions* options, FFlist* devices /* FFBluetoothResult */);

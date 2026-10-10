#pragma once

#include "fastfetch.h"
#include "modules/bluetooth/option.h"

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

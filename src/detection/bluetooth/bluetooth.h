#pragma once

#ifndef FF_INCLUDED_detection_bluetooth_bluetooth
#define FF_INCLUDED_detection_bluetooth_bluetooth

#include "fastfetch.h"

typedef struct FFBluetoothDevice
{
    FFstrbuf name;
    FFstrbuf address;
    FFstrbuf type;
    uint8_t battery; // 0-100%
    bool connected;
} FFBluetoothDevice;

typedef struct FFBluetoothResult
{
    FFstrbuf error;
    FFlist devices; // List of FFBluetoothDevice
} FFBluetoothResult;

const FFBluetoothResult* ffDetectBluetooth(const FFinstance* instance);

#endif

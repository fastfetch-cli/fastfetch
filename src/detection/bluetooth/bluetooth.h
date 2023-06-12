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

const char* ffDetectBluetooth(const FFinstance* instance, FFlist* devices /* FFBluetoothDevice */);

#endif

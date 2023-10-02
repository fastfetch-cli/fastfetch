#pragma once

#ifndef FF_INCLUDED_detection_bluetooth_bluetooth
#define FF_INCLUDED_detection_bluetooth_bluetooth

#include "fastfetch.h"

typedef struct FFBluetoothResult
{
    FFstrbuf name;
    FFstrbuf address;
    FFstrbuf type;
    uint8_t battery; // 0-100%
    bool connected;
} FFBluetoothResult;

const char* ffDetectBluetooth(FFlist* devices /* FFBluetoothResult */);

#endif

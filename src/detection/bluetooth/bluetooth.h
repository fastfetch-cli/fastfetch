#pragma once

#ifndef FF_INCLUDED_detection_bluetooth_bluetooth
#define FF_INCLUDED_detection_bluetooth_bluetooth

#include "fastfetch.h"

typedef struct FFBluetoothResult
{
    FFstrbuf error;
    FFstrbuf name;
    FFstrbuf address;
    FFstrbuf type;
    uint8_t battery; // 0-100%
} FFBluetoothResult;

const FFBluetoothResult* ffDetectBluetooth(const FFinstance* instance);

#endif

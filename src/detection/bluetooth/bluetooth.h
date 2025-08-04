#pragma once

#include "fastfetch.h"
#include "modules/bluetooth/option.h"

typedef struct FFBluetoothResult
{
    FFstrbuf name;
    FFstrbuf address;
    FFstrbuf type;
    uint8_t battery; // 0-100%
    bool connected;
} FFBluetoothResult;

const char* ffDetectBluetooth(FFBluetoothOptions* options, FFlist* devices /* FFBluetoothResult */);

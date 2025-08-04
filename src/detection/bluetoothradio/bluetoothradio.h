#pragma once

#include "fastfetch.h"
#include "modules/bluetoothradio/option.h"

typedef struct FFBluetoothRadioResult
{
    FFstrbuf name;
    FFstrbuf address;
    FFstrbuf vendor;
    int32_t lmpVersion;
    int32_t lmpSubversion;
    bool enabled;
    bool discoverable;
    bool connectable;
} FFBluetoothRadioResult;

const char* ffDetectBluetoothRadio(FFlist* devices /* FFBluetoothRadioResult */);
const char* ffBluetoothRadioGetVendor(uint32_t manufacturerId);

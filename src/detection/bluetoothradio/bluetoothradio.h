#pragma once

#include "fastfetch.h"

typedef struct FFBluetoothRadioResult
{
    FFstrbuf name;
    FFstrbuf address;
    uint8_t lmpVersion;
    uint8_t hciVersion;
    uint16_t lmpSubversion;
    uint16_t hciRevision;
    const char* vendor;
    bool leSupported;
} FFBluetoothRadioResult;

const char* ffDetectBluetoothRadio(FFlist* devices /* FFBluetoothRadioResult */);

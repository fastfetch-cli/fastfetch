#pragma once

#include "common/option.h"
#include "common/percent.h"

// Which radio a device answers on. A dual-mode device answers on both, and a backend that walks only
// one stack can only report the bit it knows about, so this is a bitfield rather than a value. It
// carries both meanings the module needs: which stacks a result was seen on, and which stacks
// `options->showType` asks the detection to walk at all.
typedef enum FFBluetoothDeviceType: uint8_t {
    FF_BLUETOOTH_DEVICE_TYPE_NONE = 0,
    FF_BLUETOOTH_DEVICE_TYPE_CLASSIC_BIT = 1 << 0, // BR/EDR, the BTHENUM enumerator
    FF_BLUETOOTH_DEVICE_TYPE_LE_BIT = 1 << 1,      // Low Energy, the BTHLE enumerator
    FF_BLUETOOTH_DEVICE_TYPE_BOTH = FF_BLUETOOTH_DEVICE_TYPE_CLASSIC_BIT | FF_BLUETOOTH_DEVICE_TYPE_LE_BIT,
} FFBluetoothDeviceType;

typedef struct FFBluetoothOptions {
    FFModuleArgs moduleArgs;

    bool showDisconnected;
    FFBluetoothDeviceType showType;
    FFPercentageModuleConfig percent;
} FFBluetoothOptions;

static_assert(sizeof(FFBluetoothOptions) <= FF_OPTION_MAX_SIZE, "FFBluetoothOptions size exceeds maximum allowed size");

#pragma once

#include "fastfetch.h"

#define FF_BLUETOOTH_MODULE_NAME "Bluetooth"

void ffPrintBluetooth(FFBluetoothOptions* options);
void ffInitBluetoothOptions(FFBluetoothOptions* options);
void ffDestroyBluetoothOptions(FFBluetoothOptions* options);

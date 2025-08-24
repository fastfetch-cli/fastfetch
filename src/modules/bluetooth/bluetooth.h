#pragma once

#include "option.h"

#define FF_BLUETOOTH_MODULE_NAME "Bluetooth"

bool ffPrintBluetooth(FFBluetoothOptions* options);
void ffInitBluetoothOptions(FFBluetoothOptions* options);
void ffDestroyBluetoothOptions(FFBluetoothOptions* options);

extern FFModuleBaseInfo ffBluetoothModuleInfo;

#pragma once

#include "option.h"

bool ffPrintBluetooth(FFBluetoothOptions* options);
void ffInitBluetoothOptions(FFBluetoothOptions* options);
void ffDestroyBluetoothOptions(FFBluetoothOptions* options);

extern FFModuleBaseInfo ffBluetoothModuleInfo;

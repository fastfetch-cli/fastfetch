#pragma once

#include "fastfetch.h"

#define FF_BLUETOOTH_MODULE_NAME "Bluetooth"

void ffPrintBluetooth(FFBluetoothOptions* options);
void ffInitBluetoothOptions(FFBluetoothOptions* options);
bool ffParseBluetoothCommandOptions(FFBluetoothOptions* options, const char* key, const char* value);
void ffDestroyBluetoothOptions(FFBluetoothOptions* options);
void ffParseBluetoothJsonObject(FFBluetoothOptions* options, yyjson_val* module);

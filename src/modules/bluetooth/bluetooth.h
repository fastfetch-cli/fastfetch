#pragma once

#include "fastfetch.h"
#include <yyjson.h>

#define FF_BLUETOOTH_MODULE_NAME "Bluetooth"

void ffPrintBluetooth(FFinstance* instance, FFBluetoothOptions* options);
void ffInitBluetoothOptions(FFBluetoothOptions* options);
bool ffParseBluetoothCommandOptions(FFBluetoothOptions* options, const char* key, const char* value);
void ffDestroyBluetoothOptions(FFBluetoothOptions* options);
void ffParseBluetoothJsonObject(FFinstance* instance, yyjson_val* module);

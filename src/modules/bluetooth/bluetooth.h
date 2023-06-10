#pragma once

#include "fastfetch.h"

#define FF_BLUETOOTH_MODULE_NAME "Bluetooth"

void ffPrintBluetooth(FFinstance* instance, FFBluetoothOptions* options);
void ffInitBluetoothOptions(FFBluetoothOptions* options);
bool ffParseBluetoothCommandOptions(FFBluetoothOptions* options, const char* key, const char* value);
void ffDestroyBluetoothOptions(FFBluetoothOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseBluetoothJsonObject(FFinstance* instance, json_object* module);
#endif

#pragma once

#include "fastfetch.h"

void ffPrintBluetooth(FFinstance* instance, FFBluetoothOptions* options);
void ffInitBluetoothOptions(FFBluetoothOptions* options);
bool ffParseBluetoothCommandOptions(FFBluetoothOptions* options, const char* key, const char* value);
void ffDestroyBluetoothOptions(FFBluetoothOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/config.h"
bool ffParseBluetoothJsonObject(FFinstance* instance, const char* type, JSONCData* data, json_object* module);
#endif

#pragma once

#include "fastfetch.h"

#define FF_BLUETOOTHRADIO_MODULE_NAME "BluetoothRadio"

void ffPrintBluetoothRadio(FFBluetoothRadioOptions* options);
void ffInitBluetoothRadioOptions(FFBluetoothRadioOptions* options);
void ffDestroyBluetoothRadioOptions(FFBluetoothRadioOptions* options);

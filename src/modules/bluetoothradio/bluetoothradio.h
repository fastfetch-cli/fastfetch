#pragma once

#include "option.h"

#define FF_BLUETOOTHRADIO_MODULE_NAME "BluetoothRadio"

bool ffPrintBluetoothRadio(FFBluetoothRadioOptions* options);
void ffInitBluetoothRadioOptions(FFBluetoothRadioOptions* options);
void ffDestroyBluetoothRadioOptions(FFBluetoothRadioOptions* options);

extern FFModuleBaseInfo ffBluetoothRadioModuleInfo;

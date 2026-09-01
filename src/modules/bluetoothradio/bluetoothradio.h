#pragma once

#include "option.h"

bool ffPrintBluetoothRadio(FFBluetoothRadioOptions* options);
void ffInitBluetoothRadioOptions(FFBluetoothRadioOptions* options);
void ffDestroyBluetoothRadioOptions(FFBluetoothRadioOptions* options);

extern FFModuleBaseInfo ffBluetoothRadioModuleInfo;

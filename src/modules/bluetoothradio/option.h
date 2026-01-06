#pragma once

#include "util/option.h"
#include "util/percent.h"

typedef struct FFBluetoothRadioOptions
{
    FFModuleArgs moduleArgs;
} FFBluetoothRadioOptions;

static_assert(sizeof(FFBluetoothRadioOptions) <= FF_OPTION_MAX_SIZE, "FFBluetoothRadioOptions size exceeds maximum allowed size");

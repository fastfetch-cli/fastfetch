#pragma once

#include "common/option.h"
#include "common/percent.h"

typedef struct FFBluetoothOptions
{
    FFModuleArgs moduleArgs;

    bool showDisconnected;
    FFPercentageModuleConfig percent;
} FFBluetoothOptions;

static_assert(sizeof(FFBluetoothOptions) <= FF_OPTION_MAX_SIZE, "FFBluetoothOptions size exceeds maximum allowed size");

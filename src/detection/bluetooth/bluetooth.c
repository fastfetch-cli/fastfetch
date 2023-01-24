#include "bluetooth.h"

#include "../internal.h"

void ffDetectBluetoothImpl(const FFinstance* instance, FFBluetoothResult* bluetooth);

const FFBluetoothResult* ffDetectBluetooth(const FFinstance* instance)
{
    FF_DETECTION_INTERNAL_GUARD(FFBluetoothResult,
        ffStrbufInit(&result.error);
        ffListInit(&result.devices, sizeof(FFBluetoothDevice));

        ffDetectBluetoothImpl(instance, &result);
    )
}

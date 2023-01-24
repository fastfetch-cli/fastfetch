#include "bluetooth.h"

#include "../internal.h"

void ffDetectBluetoothImpl(const FFinstance* instance, FFBluetoothResult* bluetooth);

const FFBluetoothResult* ffDetectBluetooth(const FFinstance* instance)
{
    FF_DETECTION_INTERNAL_GUARD(FFBluetoothResult,
        ffStrbufInit(&result.error);
        ffStrbufInit(&result.name);
        ffStrbufInit(&result.address);
        ffStrbufInit(&result.type);
        result.battery = 0;

        ffDetectBluetoothImpl(instance, &result);

        if(result.error.length == 0 && result.name.length == 0)
            ffStrbufAppendS(&result.error, "No bluetooth device found");
    )
}

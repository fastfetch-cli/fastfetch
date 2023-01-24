#include "bluetooth.h"

void ffDetectBluetoothImpl(const FFinstance* instance, FFBluetoothResult* bluetooth)
{
    ffStrbufAppendS(&bluetooth->error, "Bluetooth not supported on this platform");
}

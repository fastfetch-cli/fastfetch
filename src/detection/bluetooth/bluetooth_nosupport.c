#include "bluetooth.h"

void ffDetectBluetoothImpl(FF_MAYBE_UNUSED const FFinstance* instance, FFBluetoothResult* bluetooth)
{
    ffStrbufAppendS(&bluetooth->error, "Bluetooth not supported on this platform");
}

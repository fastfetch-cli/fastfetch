#include "bluetooth.h"

const char* ffDetectBluetooth(FF_A_UNUSED FFBluetoothOptions* options, FF_A_UNUSED FFlist* devices /* FFBluetoothResult */) {
    return "Not supported on this platform";
}

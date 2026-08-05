#include "bluetooth.h"

const char* ffDetectBluetooth([[maybe_unused]] FFBluetoothOptions* options, [[maybe_unused]] FFlist* devices /* FFBluetoothResult */) {
    return "Not supported on this platform";
}

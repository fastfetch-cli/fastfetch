#include "mouse.h"

const char* ffDetectMouse(FF_A_UNUSED FFlist* devices /* List of FFMouseDevice */) {
    return "No mouse support on this platform";
}

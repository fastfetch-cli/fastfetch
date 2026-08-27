#include "mouse.h"

const char* ffDetectMouse([[maybe_unused]] FFlist* devices /* List of FFMouseDevice */) {
    return "No mouse support on this platform";
}

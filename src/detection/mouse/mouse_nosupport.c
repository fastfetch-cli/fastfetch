#include "mouse.h"

const char* ffDetectMouse(FF_MAYBE_UNUSED FFlist* devices /* List of FFMouseDevice */)
{
    return "No mouse support on this platform";
}

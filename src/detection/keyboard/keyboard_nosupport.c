#include "keyboard.h"

const char* ffDetectKeyboard(FF_MAYBE_UNUSED FFlist* devices /* List of FFKeyboardDevice */)
{
    return "No mouse support on this platform";
}

#include "keyboard.h"

const char* ffDetectKeyboard(FF_A_UNUSED FFlist* devices /* List of FFKeyboardDevice */) {
    return "No mouse support on this platform";
}

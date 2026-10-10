#include "keyboard.h"

const char* ffDetectKeyboard([[maybe_unused]] FFlist* devices /* List of FFKeyboardDevice */) {
    return "No keyboard support on this platform";
}

#include "fastfetch.h"

typedef struct FFKeyboardDevice
{
    FFstrbuf serial;
    FFstrbuf name;
} FFKeyboardDevice;

const char* ffDetectKeyboard(FFlist* devices /* List of FFKeyboardDevice */);

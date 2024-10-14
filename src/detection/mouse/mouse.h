#include "fastfetch.h"

typedef struct FFMouseDevice
{
    FFstrbuf serial;
    FFstrbuf name;
} FFMouseDevice;

const char* ffDetectMouse(FFlist* devices /* List of FFMouseDevice */);

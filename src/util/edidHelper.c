#include "edidHelper.h"

void ffEdidGetNativeResolution(uint8_t edid[128], uint32_t* width, uint32_t* height)
{
    const int dtd = 54;
    *width = ((edid[dtd + 4] >> 4) << 8) | edid[dtd + 2];
    *height = ((edid[dtd + 7] >> 4) << 8) | edid[dtd + 5];
}

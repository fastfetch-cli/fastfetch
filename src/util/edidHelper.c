#include "edidHelper.h"

void ffEdidGetPhycialResolution(const uint8_t edid[128], uint32_t* width, uint32_t* height)
{
    const int dtd = 54;
    *width = (((uint32_t) edid[dtd + 4] >> 4) << 8) | edid[dtd + 2];
    *height = (((uint32_t) edid[dtd + 7] >> 4) << 8) | edid[dtd + 5];
}

void ffEdidGetName(const uint8_t edid[128], FFstrbuf* name)
{
    // https://github.com/jinksong/read_edid/blob/master/parse-edid/parse-edid.c
    for (uint32_t i = 0x36; i < 0x7E; i += 0x12)
    { // read through descriptor blocks...
        if (edid[i] == 0x00)
        { // not a timing descriptor
            if (edid[i+3] == 0xfc)
            { // Model Name tag
                for (uint32_t j = 0; j < 13; j++)
                {
                    if (edid[i + 5 + j] == 0x0a)
                        return;
                    ffStrbufAppendC(name, (char) edid[i + 5 + j]);
                }
            }
        }
    }
}

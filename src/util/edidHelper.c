#include "edidHelper.h"

void ffEdidGetPhysicalResolution(const uint8_t edid[128], uint32_t* width, uint32_t* height)
{
    const int dtd = 54;
    *width = (((uint32_t) edid[dtd + 4] >> 4) << 8) | edid[dtd + 2];
    *height = (((uint32_t) edid[dtd + 7] >> 4) << 8) | edid[dtd + 5];
}

void ffEdidGetVendorAndModel(const uint8_t edid[128], FFstrbuf* result)
{
    // https://github.com/jinksong/read_edid/blob/master/parse-edid/parse-edid.c
    ffStrbufAppendF(result, "%c%c%c%04X",
        (char) (((uint32_t)edid[8] >> 2 & 0x1f) + 'A' - 1),
        (char) (((((uint32_t)edid[8] & 0x3) << 3) | (((uint32_t)edid[9] & 0xe0) >> 5)) + 'A' - 1),
        (char) (((uint32_t)edid[9] & 0x1f) + 'A' - 1),
        (uint32_t) (edid[10] + (uint32_t) (edid[11] << 8))
    );
}

bool ffEdidGetName(const uint8_t edid[128], FFstrbuf* name)
{
    // https://github.com/jinksong/read_edid/blob/master/parse-edid/parse-edid.c
    for (uint32_t i = 0x36; i < 0x7E; i += 0x12)
    { // read through descriptor blocks...
        if (edid[i] == 0x00)
        { // not a timing descriptor
            if (edid[i + 3] == 0xfc)
            { // Model Name tag
                for (uint32_t j = 0; j < 13; j++)
                {
                    if (edid[i + 5 + j] == 0x0a)
                    {
                        ffStrbufAppendNS(name, j, (const char*) &edid[i + 5]);
                        return true;
                    }
                }
            }
        }
    }

    // use manufacturer + model number as monitor name
    ffEdidGetVendorAndModel(edid, name);
    return false;
}

void ffEdidGetPhysicalSize(const uint8_t edid[128], uint32_t* width, uint32_t* height)
{
    *width = (((uint32_t) edid[68] & 0xF0) << 4) + edid[66];
    *height = (((uint32_t) edid[68] & 0x0F) << 8) + edid[67];
}

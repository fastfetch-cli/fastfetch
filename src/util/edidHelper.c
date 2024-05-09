#include "edidHelper.h"

void ffEdidGetPhysicalResolution(const uint8_t edid[128], uint32_t* width, uint32_t* height)
{
    const int dtd = 54;
    *width = (((uint32_t) edid[dtd + 4] >> 4) << 8) | edid[dtd + 2];
    *height = (((uint32_t) edid[dtd + 7] >> 4) << 8) | edid[dtd + 5];
}

void ffEdidGetPreferredResolutionAndRefreshRate(const uint8_t edid[128], uint32_t* width, uint32_t* height, double* refreshRate)
{
    for (uint32_t i = 0x36; i < 0x7E; i += 0x12)
    { // read through descriptor blocks...
        if (edid[i] != 0x00 && edid[i + 1] != 0x00)
        { // a dtd
            uint32_t hactive = edid[i+2] + (uint32_t) ((edid[i + 4] & 0xf0) << 4);
            uint32_t hblank = edid[i+3] + (uint32_t) ((edid[i + 4] & 0x0f) << 8);
            uint32_t vactive = edid[i+5] + (uint32_t) ((edid[i + 7] & 0xf0) << 4);
            uint32_t vblank = edid[i+6] + (uint32_t) ((edid[i + 7] & 0x0f) << 8);
            uint32_t pixclk = ((uint32_t) edid[i+1] << 8) | (edid[i]);
            *width = hactive;
            *height = vactive;
            *refreshRate = (double)pixclk * 10000 / (double)(hactive+hblank) / (double)(vactive+vblank);
            return;
        }
    }
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

void ffEdidGetSerialAndManufactureDate(const uint8_t edid[128], uint32_t* serial, uint16_t* year, uint16_t* week)
{
    if (edid[17] > 0 && edid[17] < 0xFF)
    {
        *year = (uint16_t) edid[17] + 1990;
        *week = (uint16_t) edid[16];
        if (*week == 0xFF) *week = 0;
    }
    else
        *year = *week = 0;

    *serial = *(uint32_t*) &edid[12];
}

bool ffEdidGetHdrCompatible(const uint8_t* edid, uint32_t length)
{
    if (length <= 128) return false;
    for (const uint8_t* cta = &edid[128]; cta < &edid[length]; cta += 128)
    {
        // https://en.wikipedia.org/wiki/Extended_Display_Identification_Data#CTA_EDID_Timing_Extension_Block
        if (cta[0] != 0x02 /* CTA EDID */) continue;
        if (cta[1] < 0x03 /* Version 3 */) continue;
        const uint8_t offset = cta[2];
        if (offset <= 4) continue;
        for (uint8_t i = 4; i < offset;)
        {
            uint8_t blkLen = cta[i] & 0x1f;
            if (blkLen > 0)
            {
                uint8_t blkTag = (cta[i] & 0xe0) >> 5;
                if (blkTag == 0x07 /* Extended Block Type Tag */)
                {
                    uint8_t extendedTag = cta[i + 1];
                    if (extendedTag == 6 /* HDR SMDB */ || extendedTag == 7 /* HDR DMDB */)
                        return true;
                }
            }
            i += (uint8_t) (blkLen + 1);
        }
    }
    return false;
}

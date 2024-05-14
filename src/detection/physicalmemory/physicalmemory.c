#include "physicalmemory.h"

static inline const char* getVendorString(unsigned vendorId)
{
    switch (vendorId)
    {
        case 0x017A: return "Apacer";
        case 0x0198: return "Kingston";
        case 0x029E: return "Corsair";
        case 0x04CB: return "A-DATA";
        case 0x04CD: return "G-Skill";
        case 0x059B: case 0x859B: return "Crucial";
        case 0x00CE: case 0x80CE: case 0xCE00: return "Samsung";
        case 0x014F: return "Transcend";
        case 0x2C00: case 0x802C: return "Micron";
        case 0xAD00: case 0x80AD: return "SK Hynix";
        case 0x5105: case 0x8551: return "Qimonda";
        case 0x02FE: return "Elpida";
        case 0x0467: return "Ramaxel";
        default: return NULL;
    }
}

void FFPhysicalMemoryUpdateVendorString(FFPhysicalMemoryResult* device)
{
    char vendorIdStr[5];
    if (ffStrbufStartsWithS(&device->vendor, "0x"))
    {
        if (device->vendor.length < 6) return;
        memcpy(vendorIdStr, device->vendor.chars + 2, 4);
    }
    else
    {
        if (device->vendor.length < 4) return;
        memcpy(vendorIdStr, device->vendor.chars, 4);
    }
    vendorIdStr[4] = '\0';
    char* pEnd = NULL;
    uint32_t vendorId = (uint32_t) strtoul(vendorIdStr, &pEnd, 16);
    if (*pEnd != '\0') return;
    const char* vendorStr = getVendorString(vendorId);
    if (vendorStr) ffStrbufSetStatic(&device->vendor, vendorStr);
}

#include "gpu.h"
#include "common/io/io.h"
#include "common/properties.h"

#include <stdlib.h>
#ifdef __FreeBSD__
    #include <paths.h>
    #ifndef _PATH_LOCALBASE
        #define _PATH_LOCALBASE "/usr/local"
    #endif
#elif __OpenBSD__
    #define _PATH_LOCALBASE "/usr/local"
#elif __NetBSD__
    #define _PATH_LOCALBASE "/usr/pkg"
#endif

#if FF_HAVE_EMBEDDED_PCIIDS
#include "fastfetch_pciids.c.inc"
#endif
#if FF_HAVE_EMBEDDED_AMDGPUIDS
#include "fastfetch_amdgpuids.c.inc"
#endif

#define FF_STR_INDIR(x) #x
#define FF_STR(x) FF_STR_INDIR(x)

static const FFstrbuf* loadPciIds()
{
    static FFstrbuf pciids;

    if (pciids.chars) return &pciids;
    ffStrbufInit(&pciids);

    #ifdef FF_CUSTOM_PCI_IDS_PATH

        ffReadFileBuffer(FF_STR(FF_CUSTOM_PCI_IDS_PATH), &pciids);

    #else // FF_CUSTOM_PCI_IDS_PATH

        #if __linux__
        ffReadFileBuffer(FASTFETCH_TARGET_DIR_USR "/share/hwdata/pci.ids", &pciids);
        if (pciids.length == 0)
        {
            ffReadFileBuffer(FASTFETCH_TARGET_DIR_USR "/share/misc/pci.ids", &pciids); // debian?
            if (pciids.length == 0)
                ffReadFileBuffer(FASTFETCH_TARGET_DIR_USR "/local/share/hwdata/pci.ids", &pciids);
        }
        #elif __OpenBSD__ || __FreeBSD__ || __NetBSD__
        ffReadFileBuffer(_PATH_LOCALBASE "/share/hwdata/pci.ids", &pciids);
        if (pciids.length == 0)
            ffReadFileBuffer(_PATH_LOCALBASE "/share/pciids/pci.ids", &pciids);
        #elif __sun
        ffReadFileBuffer(FASTFETCH_TARGET_DIR_ROOT "/usr/share/hwdata/pci.ids", &pciids);
        #elif __HAIKU__
        ffReadFileBuffer(FASTFETCH_TARGET_DIR_ROOT "/system/data/hwdata/pci.ids", &pciids);
        #endif

    #endif // FF_CUSTOM_PCI_IDS_PATH

    return &pciids;
}

static void parsePciIdsFile(const FFstrbuf* content, uint8_t subclass, uint16_t vendor, uint16_t device, FFGPUResult* gpu)
{
    if (content->length)
    {
        char buffer[32];

        // Search for vendor
        uint32_t len = (uint32_t) snprintf(buffer, ARRAY_SIZE(buffer), "\n%04x  ", vendor);
        char* start = (char*) memmem(content->chars, content->length, buffer, len);
        char* end = content->chars + content->length;
        if (start)
        {
            start += len;
            end = memchr(start, '\n', (uint32_t) (end - start));
            if (!end)
                end = content->chars + content->length;
            if (!gpu->vendor.length)
                ffStrbufSetNS(&gpu->vendor, (uint32_t) (end - start), start);

            start = end; // point to '\n' of vendor
            end = start + 1; // point to start of devices
            // find the start of next vendor
            while (end[0] == '\t' || end[0] == '#')
            {
                end = strchr(end, '\n');
                if (!end)
                {
                    end = content->chars + content->length;
                    break;
                }
                else
                    end++;
            }

            // Search for device
            len = (uint32_t) snprintf(buffer, ARRAY_SIZE(buffer), "\n\t%04x  ", device);
            start = memmem(start, (size_t) (end - start), buffer, len);
            if (start)
            {
                start += len;
                end = memchr(start, '\n', (uint32_t) (end - start));
                if (!end)
                    end = content->chars + content->length;

                char* closingBracket = end - 1;
                if (*closingBracket == ']')
                {
                    char* openingBracket = memrchr(start, '[', (size_t) (closingBracket - start));
                    if (openingBracket)
                    {
                        openingBracket++;
                        ffStrbufSetNS(&gpu->name, (uint32_t) (closingBracket - openingBracket), openingBracket);
                    }
                }
                if (!gpu->name.length)
                    ffStrbufSetNS(&gpu->name, (uint32_t) (end - start), start);
            }
        }
    }

    if (!gpu->name.length)
    {
        const char* subclassStr;
        switch (subclass)
        {
        case 0 /*PCI_CLASS_DISPLAY_VGA*/: subclassStr = " (VGA compatible)"; break;
        case 1 /*PCI_CLASS_DISPLAY_XGA*/: subclassStr = " (XGA compatible)"; break;
        case 2 /*PCI_CLASS_DISPLAY_3D*/: subclassStr = " (3D)"; break;
        default: subclassStr = ""; break;
        }

        ffStrbufSetF(&gpu->name, "%s Device %04X%s", gpu->vendor.length ? gpu->vendor.chars : "Unknown", device, subclassStr);
    }
}

#if FF_HAVE_EMBEDDED_PCIIDS
static inline int pciDeviceCmp(const uint16_t* key, const FFPciDevice* element)
{
    return (int) *key - (int) element->id;
}

static bool loadPciidsInc(uint8_t subclass, uint16_t vendor, uint16_t device, FFGPUResult* gpu)
{
    for (const FFPciVendor* pvendor = ffPciVendors; pvendor->name; pvendor++)
    {
        if (pvendor->id != vendor) continue;

        if (!gpu->vendor.length)
            ffStrbufSetS(&gpu->vendor, pvendor->name);

        const FFPciDevice* pdevice = (const FFPciDevice*) bsearch(&device, pvendor->devices, pvendor->nDevices, sizeof(*pdevice), (void*) pciDeviceCmp);

        if (pdevice)
        {
            uint32_t nameLen = (uint32_t) strlen(pdevice->name);
            const char* closingBracket = pdevice->name + nameLen - 1;
            if (*closingBracket == ']')
            {
                const char* openingBracket = memrchr(pdevice->name, '[', nameLen - 1);
                if (openingBracket)
                {
                    openingBracket++;
                    ffStrbufSetNS(&gpu->name, (uint32_t) (closingBracket - openingBracket), openingBracket);
                }
            }
            if (!gpu->name.length)
                ffStrbufSetNS(&gpu->name, nameLen, pdevice->name);
            return true;
        }

        if (!gpu->name.length)
        {
            const char* subclassStr;
            switch (subclass)
            {
            case 0 /*PCI_CLASS_DISPLAY_VGA*/: subclassStr = " (VGA compatible)"; break;
            case 1 /*PCI_CLASS_DISPLAY_XGA*/: subclassStr = " (XGA compatible)"; break;
            case 2 /*PCI_CLASS_DISPLAY_3D*/: subclassStr = " (3D)"; break;
            default: subclassStr = ""; break;
            }

            ffStrbufSetF(&gpu->name, "%s Device %04X%s", gpu->vendor.length ? gpu->vendor.chars : "Unknown", device, subclassStr);
        }
        return true;
    }
    return false;
}
#endif

void ffGPUFillVendorAndName(uint8_t subclass, uint16_t vendor, uint16_t device, FFGPUResult* gpu)
{
    #if FF_HAVE_EMBEDDED_PCIIDS
    bool ok = loadPciidsInc(subclass, vendor, device, gpu);
    if (ok) return;
    #endif
    return parsePciIdsFile(loadPciIds(), subclass, vendor, device, gpu);
}

#if FF_HAVE_EMBEDDED_AMDGPUIDS
static inline int amdGpuCmp(const uint32_t* key, const FFArmGpuProduct* element)
{
    // Maximum value of *key is 0x00FFFFFF. `(int) *key` should never overflow
    return (int) *key - (int) element->id;
}

static bool loadAmdGpuIdsInc(uint16_t deviceId, uint8_t revision, FFGPUResult* gpu)
{
    uint32_t key = (deviceId << 8u) | revision;
    FFArmGpuProduct* product = bsearch(&key, ffAmdGpuProducts, ARRAY_SIZE(ffAmdGpuProducts), sizeof(*ffAmdGpuProducts), (void*) amdGpuCmp);
    if (product)
    {
        ffStrbufSetS(&gpu->name, product->name);
        return true;
    }
    return false;
}
#endif

static void parseAmdGpuIdsFile(uint16_t deviceId, uint8_t revision, FFGPUResult* gpu)
{
    char query[32];
    snprintf(query, ARRAY_SIZE(query), "%X,\t%X,", (unsigned) deviceId, (unsigned) revision);
    #ifdef FF_CUSTOM_AMDGPU_IDS_PATH
    ffParsePropFile(FF_STR(FF_CUSTOM_AMDGPU_IDS_PATH), query, &gpu->name);
    #else
    ffParsePropFileData("libdrm/amdgpu.ids", query, &gpu->name);
    #endif
}

void ffGPUQueryAmdGpuName(uint16_t deviceId, uint8_t revisionId, FFGPUResult* gpu)
{
    #if FF_HAVE_EMBEDDED_AMDGPUIDS
    bool ok = loadAmdGpuIdsInc(deviceId, revisionId, gpu);
    if (ok) return;
    #endif
    return parseAmdGpuIdsFile(deviceId, revisionId, gpu);
}

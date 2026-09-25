#include "gpu.h"
#include "common/debug.h"
#include "common/io.h"
#include "common/path.h"
#include "common/properties.h"
#include "common/memrchr.h"
#include "common/strutil.h"

#if FF_HAVE_EMBEDDED_PCIIDS
    #include "fastfetch_pciids.c.inc"
#endif
#if FF_HAVE_EMBEDDED_AMDGPUIDS
    #include "fastfetch_amdgpuids.c.inc"
#endif

static const FFstrbuf* loadPciIds() {
    static FFstrbuf pciids;

    if (pciids.chars) {
        return &pciids;
    }
    ffStrbufInit(&pciids);

#ifdef FF_CUSTOM_PCI_IDS_PATH

    FF_DEBUG("Loading PCI IDs from custom path: %s", FF_STR(FF_CUSTOM_PCI_IDS_PATH));
    ffReadFileBuffer(FF_STR(FF_CUSTOM_PCI_IDS_PATH), &pciids);

#else // FF_CUSTOM_PCI_IDS_PATH

    #if __linux__
    FF_DEBUG("Loading PCI IDs from %s", FASTFETCH_TARGET_DIR_USR "/share/hwdata/pci.ids");
    ffReadFileBuffer(FASTFETCH_TARGET_DIR_USR "/share/hwdata/pci.ids", &pciids);
    if (pciids.length == 0) {
        FF_DEBUG("PCI IDs not found, trying %s", FASTFETCH_TARGET_DIR_USR "/share/misc/pci.ids");
        ffReadFileBuffer(FASTFETCH_TARGET_DIR_USR "/share/misc/pci.ids", &pciids); // debian?
        if (pciids.length == 0) {
            FF_DEBUG("PCI IDs not found, trying %s", FASTFETCH_TARGET_DIR_USR "/local/share/hwdata/pci.ids");
            ffReadFileBuffer(FASTFETCH_TARGET_DIR_USR "/local/share/hwdata/pci.ids", &pciids);
        }
    }
    #elif __OpenBSD__ || __FreeBSD__ || __NetBSD__
    FF_DEBUG("Loading PCI IDs from %s", FF_PATH_PKG_BASE "/share/hwdata/pci.ids");
    ffReadFileBuffer(FF_PATH_PKG_BASE "/share/hwdata/pci.ids", &pciids);
    if (pciids.length == 0) {
        FF_DEBUG("PCI IDs not found, trying %s", FF_PATH_PKG_BASE "/share/pciids/pci.ids");
        ffReadFileBuffer(FF_PATH_PKG_BASE "/share/pciids/pci.ids", &pciids);
    }
    #elif __sun
    FF_DEBUG("Loading PCI IDs from %s", FASTFETCH_TARGET_DIR_ROOT "/usr/share/hwdata/pci.ids");
    ffReadFileBuffer(FASTFETCH_TARGET_DIR_ROOT "/usr/share/hwdata/pci.ids", &pciids);
    #elif __HAIKU__
    FF_DEBUG("Loading PCI IDs from %s", FASTFETCH_TARGET_DIR_ROOT "/system/data/hwdata/pci.ids");
    ffReadFileBuffer(FASTFETCH_TARGET_DIR_ROOT "/system/data/hwdata/pci.ids", &pciids);
    #endif

#endif // FF_CUSTOM_PCI_IDS_PATH

    FF_DEBUG("PCI IDs data loaded: %u bytes", pciids.length);
    return &pciids;
}

static void parsePciIdsFile(const FFstrbuf* content, uint8_t subclass, uint16_t vendor, uint16_t device, FFGPUResult* gpu) {
    FF_DEBUG("Searching pci.ids for vendor=0x%04x device=0x%04x subclass=0x%02x", vendor, device, subclass);
    if (content->length) {
        char buffer[32];

        // Search for vendor
        uint32_t len = (uint32_t) snprintf(buffer, ARRAY_SIZE(buffer), "\n%04x  ", vendor);
        char* start = (char*) memmem(content->chars, content->length, buffer, len);
        char* end = content->chars + content->length;
        if (start) {
            FF_DEBUG("Found PCI vendor entry for 0x%04x", vendor);
            start += len;
            end = memchr(start, '\n', (uint32_t) (end - start));
            if (!end) {
                end = content->chars + content->length;
            }
            if (!gpu->vendor.length) {
                ffStrbufSetNS(&gpu->vendor, (uint32_t) (end - start), start);
            }

            start = end;     // point to '\n' of vendor
            end = start + 1; // point to start of devices
            // find the start of next vendor
            while (end[0] == '\t' || end[0] == '#') {
                end = strchr(end, '\n');
                if (!end) {
                    end = content->chars + content->length;
                    break;
                } else {
                    end++;
                }
            }

            // Search for device
            len = (uint32_t) snprintf(buffer, ARRAY_SIZE(buffer), "\n\t%04x  ", device);
            start = memmem(start, (size_t) (end - start), buffer, len);
            if (start) {
                FF_DEBUG("Found PCI device entry for 0x%04x:0x%04x", vendor, device);
                start += len;
                end = memchr(start, '\n', (uint32_t) (end - start));
                if (!end) {
                    end = content->chars + content->length;
                }

                char* closingBracket = end - 1;
                if (*closingBracket == ']') {
                    char* openingBracket = memrchr(start, '[', (size_t) (closingBracket - start));
                    if (openingBracket) {
                        openingBracket++;
                        ffStrbufSetNS(&gpu->name, (uint32_t) (closingBracket - openingBracket), openingBracket);
                    }
                }
                if (!gpu->name.length) {
                    ffStrbufSetNS(&gpu->name, (uint32_t) (end - start), start);
                }
            } else {
                FF_DEBUG("PCI device entry 0x%04x:0x%04x was not found in pci.ids", vendor, device);
            }
        } else {
            FF_DEBUG("PCI vendor entry 0x%04x was not found in pci.ids", vendor);
        }
    } else {
        FF_DEBUG("PCI IDs data is empty; using a synthesized device name");
    }

    if (!gpu->name.length) {
        const char* subclassStr;
        switch (subclass) {
            case 0 /*PCI_CLASS_DISPLAY_VGA*/:
                subclassStr = " (VGA compatible)";
                break;
            case 1 /*PCI_CLASS_DISPLAY_XGA*/:
                subclassStr = " (XGA compatible)";
                break;
            case 2 /*PCI_CLASS_DISPLAY_3D*/:
                subclassStr = " (3D)";
                break;
            default:
                subclassStr = "";
                break;
        }

        ffStrbufSetF(&gpu->name, "%s Device %04X%s", gpu->vendor.length ? gpu->vendor.chars : "Unknown", device, subclassStr);
        FF_DEBUG("PCI device name not found; synthesized name: %s", gpu->name.chars);
    } else {
        FF_DEBUG("Resolved PCI device name: vendor='%s', name='%s'", gpu->vendor.chars, gpu->name.chars);
    }
}

#if FF_HAVE_EMBEDDED_PCIIDS
static inline int pciDeviceCmp(const uint16_t* key, const FFPciDevice* element) {
    return (int) *key - (int) element->id;
}

static bool loadPciidsInc(uint8_t subclass, uint16_t vendor, uint16_t device, FFGPUResult* gpu) {
    FF_DEBUG("Searching embedded PCI IDs for vendor=0x%04x device=0x%04x", vendor, device);
    for (const FFPciVendor* pvendor = ffPciVendors; pvendor->name; pvendor++) {
        if (pvendor->id != vendor) {
            continue;
        }

        if (!gpu->vendor.length) {
            ffStrbufSetS(&gpu->vendor, pvendor->name);
        }

        const FFPciDevice* pdevice = (const FFPciDevice*) bsearch(&device, pvendor->devices, pvendor->nDevices, sizeof(*pdevice), (void*) pciDeviceCmp);

        if (pdevice) {
            uint32_t nameLen = (uint32_t) strlen(pdevice->name);
            if (nameLen == 0) {
                FF_DEBUG("Embedded PCI device entry for 0x%04x:0x%04x has an empty name", vendor, device);
                return false;
            }
            const char* closingBracket = pdevice->name + nameLen - 1;
            if (*closingBracket == ']') {
                const char* openingBracket = memrchr(pdevice->name, '[', nameLen - 1);
                if (openingBracket) {
                    openingBracket++;
                    ffStrbufSetNS(&gpu->name, (uint32_t) (closingBracket - openingBracket), openingBracket);
                }
            }
            if (!gpu->name.length) {
                ffStrbufSetNS(&gpu->name, nameLen, pdevice->name);
            }
            FF_DEBUG("Embedded PCI ID matched: vendor='%s', name='%s'", gpu->vendor.chars, gpu->name.chars);
            return true;
        }

        FF_DEBUG("Embedded PCI vendor matched ('%s'), but device 0x%04x was not found", pvendor->name, device);
        if (!gpu->name.length) {
            const char* subclassStr;
            switch (subclass) {
                case 0 /*PCI_CLASS_DISPLAY_VGA*/:
                    subclassStr = " (VGA compatible)";
                    break;
                case 1 /*PCI_CLASS_DISPLAY_XGA*/:
                    subclassStr = " (XGA compatible)";
                    break;
                case 2 /*PCI_CLASS_DISPLAY_3D*/:
                    subclassStr = " (3D)";
                    break;
                default:
                    subclassStr = "";
                    break;
            }

            ffStrbufSetF(&gpu->name, "%s Device %04X%s", gpu->vendor.length ? gpu->vendor.chars : "Unknown", device, subclassStr);
        }
        FF_DEBUG("Using synthesized PCI device name: %s", gpu->name.chars);
        return true;
    }
    FF_DEBUG("Embedded PCI vendor 0x%04x was not found", vendor);
    return false;
}
#endif

void ffGPUFillVendorAndName(uint8_t subclass, uint16_t vendor, uint16_t device, FFGPUResult* gpu) {
    FF_DEBUG("Resolving PCI GPU name: vendor=0x%04x device=0x%04x subclass=0x%02x", vendor, device, subclass);
    if (vendor == 0x1234 && device == 0x1111 && subclass == 0) { // Not exist in pci.ids
        ffStrbufSetStatic(&gpu->name, "Virtual Video Controller");
        FF_DEBUG("Matched special virtual video controller ID");
        return;
    }

#if FF_HAVE_EMBEDDED_PCIIDS
    bool ok = loadPciidsInc(subclass, vendor, device, gpu);
    if (ok) {
        FF_DEBUG("Resolved PCI GPU name from embedded IDs: vendor='%s', name='%s'", gpu->vendor.chars, gpu->name.chars);
        return;
    }
#endif
    parsePciIdsFile(loadPciIds(), subclass, vendor, device, gpu);
    FF_DEBUG("Resolved PCI GPU name from pci.ids: vendor='%s', name='%s'", gpu->vendor.chars, gpu->name.chars);
}

#if FF_HAVE_EMBEDDED_AMDGPUIDS
static inline int amdGpuCmp(const uint32_t* key, const FFArmGpuProduct* element) {
    // Maximum value of *key is 0x00FFFFFF. `(int) *key` should never overflow
    return (int) *key - (int) element->id;
}

static bool loadAmdGpuIdsInc(uint16_t deviceId, uint8_t revision, FFGPUResult* gpu) {
    uint32_t key = (deviceId << 8u) | revision;
    FFArmGpuProduct* product = bsearch(&key, ffAmdGpuProducts, ARRAY_SIZE(ffAmdGpuProducts), sizeof(*ffAmdGpuProducts), (void*) amdGpuCmp);
    if (product) {
        ffStrbufSetS(&gpu->name, product->name);
        FF_DEBUG("Embedded AMD GPU ID matched: device=0x%04x revision=0x%02x name='%s'", deviceId, revision, gpu->name.chars);
        return true;
    }
    FF_DEBUG("Embedded AMD GPU ID not found: device=0x%04x revision=0x%02x", deviceId, revision);
    return false;
}
#endif

static void parseAmdGpuIdsFile(uint16_t deviceId, uint8_t revision, FFGPUResult* gpu) {
    char query[32];
    snprintf(query, ARRAY_SIZE(query), "%X,\t%X,", (unsigned) deviceId, (unsigned) revision);
#ifdef FF_CUSTOM_AMDGPU_IDS_PATH
    FF_DEBUG("Searching AMD GPU IDs at custom path '%s' for device=0x%04x revision=0x%02x", FF_STR(FF_CUSTOM_AMDGPU_IDS_PATH), deviceId, revision);
    ffParsePropFile(FF_STR(FF_CUSTOM_AMDGPU_IDS_PATH), query, &gpu->name);
#else
    FF_DEBUG("Searching libdrm/amdgpu.ids for device=0x%04x revision=0x%02x", deviceId, revision);
    ffParsePropFileData("libdrm/amdgpu.ids", query, &gpu->name);
#endif
    FF_DEBUG("AMD GPU IDs file lookup %s: device=0x%04x revision=0x%02x name='%s'", gpu->name.length ? "matched" : "not matched", deviceId, revision, gpu->name.chars);
}

void ffGPUQueryAmdGpuName(uint16_t deviceId, uint8_t revisionId, FFGPUResult* gpu) {
    FF_DEBUG("Resolving AMD GPU name: device=0x%04x revision=0x%02x", deviceId, revisionId);
#if FF_HAVE_EMBEDDED_AMDGPUIDS
    bool ok = loadAmdGpuIdsInc(deviceId, revisionId, gpu);
    if (ok) {
        return;
    }
#endif
    parseAmdGpuIdsFile(deviceId, revisionId, gpu);
}

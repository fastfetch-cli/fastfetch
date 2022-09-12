#include "gpu.h"
#include "detection/vulkan.h"

#define FF_GPU_VENDOR_NAME_AMD "AMD"
#define FF_GPU_VENDOR_NAME_INTEL "Intel"
#define FF_GPU_VENDOR_NAME_NVIDIA "NVIDIA"

#ifdef FF_HAVE_LIBPCI
#include "common/library.h"
#include "common/properties.h"
#include "common/parsing.h"
#include "detection/temps.h"
#include <string.h>
#include <unistd.h>
#include <pci/pci.h>

typedef struct PCIData
{
    struct pci_access* access;
    FF_LIBRARY_SYMBOL(pci_read_byte);
    FF_LIBRARY_SYMBOL(pci_read_word);
    FF_LIBRARY_SYMBOL(pci_lookup_name);
    FF_LIBRARY_SYMBOL(pci_get_param);
} PCIData;

static bool pciIDsContains(u16 searched, const u16* ids)
{
    while(*ids != 0)
    {
        if(*ids == searched)
            return true;
        ids++;
    }
    return false;
}

static void pciDetectVendorName(FFGPUResult* gpu, PCIData* pci, struct pci_dev* device)
{
    if(pciIDsContains(device->vendor_id, (const u16[]) {0x1002, 0x1022, 0}))
        ffStrbufAppendS(&gpu->vendor, FF_GPU_VENDOR_NAME_AMD);
    else if(pciIDsContains(device->vendor_id, (const u16[]) {0x03e7, 0x8086, 0x8087, 0}))
        ffStrbufAppendS(&gpu->vendor, FF_GPU_VENDOR_NAME_INTEL);
    else if(pciIDsContains(device->vendor_id, (const u16[]) {0x0955, 0x10de, 0x12d2, 0}))
        ffStrbufAppendS(&gpu->vendor, FF_GPU_VENDOR_NAME_NVIDIA);

    if(gpu->vendor.length > 0)
        return;

    ffStrbufEnsureFree(&gpu->vendor, 255);
    pci->ffpci_lookup_name(pci->access, gpu->vendor.chars, (int) gpu->vendor.allocated, PCI_LOOKUP_VENDOR, device->vendor_id);
    ffStrbufRecalculateLength(&gpu->vendor);

    if(ffStrbufFirstIndexS(&gpu->vendor, "AMD") < gpu->vendor.length || ffStrbufFirstIndexS(&gpu->vendor, "ATI") < gpu->vendor.length)
        ffStrbufSetS(&gpu->vendor, FF_GPU_VENDOR_NAME_AMD);
    else if(ffStrbufFirstIndexS(&gpu->vendor, "Intel") < gpu->vendor.length)
        ffStrbufSetS(&gpu->vendor, FF_GPU_VENDOR_NAME_INTEL);
    else if(ffStrbufFirstIndexS(&gpu->vendor, "NVIDIA") < gpu->vendor.length)
        ffStrbufSetS(&gpu->vendor, FF_GPU_VENDOR_NAME_NVIDIA);
}

static void drmDetectDeviceName(FFGPUResult* gpu, PCIData* pci, struct pci_dev* device)
{
    FFstrbuf query;
    ffStrbufInit(&query);
    ffStrbufAppendF(&query, "%X, %X,", device->device_id, pci->ffpci_read_byte(device, PCI_REVISION_ID));

    ffParsePropFile(FASTFETCH_TARGET_DIR_USR"/share/libdrm/amdgpu.ids", query.chars, &gpu->name);

    ffStrbufDestroy(&query);

    const char* removeStrings[] = {
        "AMD ", "ATI ",
        " (TM)", "(TM)",
        " Graphics Adapter", " Graphics", " Series", " Edition"
    };
    ffStrbufRemoveStringsA(&gpu->name, sizeof(removeStrings) / sizeof(removeStrings[0]), removeStrings);
}

static void pciDetectDeviceName(FFGPUResult* gpu, PCIData* pci, struct pci_dev* device)
{
    if(ffStrbufCompS(&gpu->vendor, FF_GPU_VENDOR_NAME_AMD) == 0)
    {
        drmDetectDeviceName(gpu, pci, device);
        if(gpu->name.length > 0)
            return;
    }

    ffStrbufEnsureFree(&gpu->name, 255);
    pci->ffpci_lookup_name(pci->access, gpu->name.chars, (int) gpu->name.allocated, PCI_LOOKUP_DEVICE, device->vendor_id, device->device_id);
    ffStrbufRecalculateLength(&gpu->name);

    uint32_t openingBracket = ffStrbufFirstIndexC(&gpu->name, '[');
    uint32_t closingBracket = ffStrbufNextIndexC(&gpu->name, openingBracket, ']');
    if(closingBracket < gpu->name.length)
    {
        ffStrbufSubstrBefore(&gpu->name, closingBracket);
        ffStrbufSubstrAfter(&gpu->name, openingBracket);
    }
}

static void pciDetectDriverName(FFGPUResult* gpu, PCIData* pci, struct pci_dev* device)
{
    const char* base = pci->ffpci_get_param(pci->access, "sysfs.path");
    if(!ffStrSet(base))
        return;

    FFstrbuf path;
    ffStrbufInitA(&path, 64);
    ffStrbufAppendF(&path, "%s/devices/%04x:%02x:%02x.%d/driver", base, device->domain, device->bus, device->dev, device->func);

    ffStrbufEnsureFree(&gpu->driver, 1023);
    ssize_t resultLength = readlink(path.chars, gpu->driver.chars, gpu->driver.allocated - 1); //-1 for null terminator
    if(resultLength > 0)
    {
        gpu->driver.length = (uint32_t) resultLength;
        gpu->driver.chars[resultLength] = '\0';
        ffStrbufSubstrAfterLastC(&gpu->driver, '/');
    }

    ffStrbufDestroy(&path);
}

static void pciDetectTemperatur(FFGPUResult* gpu, struct pci_dev* device)
{
    const FFTempsResult* tempsResult = ffDetectTemps();

    for(uint32_t i = 0; i < tempsResult->values.length; i++)
    {
        FFTempValue* tempValue = ffListGet(&tempsResult->values, i);

        uint32_t tempClass;
        if(sscanf(tempValue->deviceClass.chars, "%x", &tempClass) != 1)
            continue;

        //The kernel exposes the device class multiplied by 256 for some reason
        if(tempClass == device->device_class * 256)
        {
            gpu->temperature = tempValue->value;
            return;
        }
    }
}

static void pciHandleDevice(FFlist* results, PCIData* pci, struct pci_dev* device)
{
    device->device_class = pci->ffpci_read_word(device, PCI_CLASS_DEVICE);

    char class[1024];
    pci->ffpci_lookup_name(pci->access, class, sizeof(class) - 1, PCI_LOOKUP_CLASS, device->device_class);

    if(
        strcasecmp("VGA compatible controller", class) != 0 &&
        strcasecmp("3D controller", class)             != 0 &&
        strcasecmp("Display controller", class)        != 0
    ) return;

    device->vendor_id = pci->ffpci_read_word(device, PCI_VENDOR_ID);
    device->device_id = pci->ffpci_read_word(device, PCI_DEVICE_ID);

    FFGPUResult* gpu = ffListAdd(results);

    ffStrbufInit(&gpu->vendor);
    pciDetectVendorName(gpu, pci, device);

    ffStrbufInit(&gpu->name);
    pciDetectDeviceName(gpu, pci, device);

    ffStrbufInit(&gpu->driver);
    pciDetectDriverName(gpu, pci, device);

    gpu->temperature = FF_GPU_TEMP_UNSET;
    pciDetectTemperatur(gpu, device);
}

static void pciDetectGPUs(const FFinstance* instance, FFlist* gpus)
{
    PCIData pci;

    FF_LIBRARY_LOAD(libpci, instance->config.libPCI, , "libpci.so", 4)
    FF_LIBRARY_LOAD_SYMBOL(libpci, pci_alloc, )
    FF_LIBRARY_LOAD_SYMBOL(libpci, pci_init, )
    FF_LIBRARY_LOAD_SYMBOL(libpci, pci_scan_bus, )
    FF_LIBRARY_LOAD_SYMBOL(libpci, pci_cleanup, )

    FF_LIBRARY_LOAD_SYMBOL_VAR(libpci, pci, pci_read_byte, )
    FF_LIBRARY_LOAD_SYMBOL_VAR(libpci, pci, pci_read_word, )
    FF_LIBRARY_LOAD_SYMBOL_VAR(libpci, pci, pci_lookup_name, )
    FF_LIBRARY_LOAD_SYMBOL_VAR(libpci, pci, pci_get_param, )

    pci.access = ffpci_alloc();
    ffpci_init(pci.access);
    ffpci_scan_bus(pci.access);

    struct pci_dev* device = pci.access->devices;
    while(device != NULL)
    {
        pciHandleDevice(gpus, &pci, device);
        device = device->next;
    }

    ffpci_cleanup(pci.access);
    dlclose(libpci);
}

#endif

void ffDetectGPUImpl(FFlist* gpus, const FFinstance* instance)
{
    #ifdef FF_HAVE_LIBPCI
        pciDetectGPUs(instance, gpus);
    #endif
}

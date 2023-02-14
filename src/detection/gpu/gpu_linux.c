#include "detection/gpu/gpu.h"
#include "detection/vulkan/vulkan.h"

#ifdef FF_HAVE_LIBPCI
#include "common/library.h"
#include "common/properties.h"
#include "common/parsing.h"
#include "detection/temps/temps_linux.h"
#include "util/stringUtils.h"
#include <string.h>
#include <unistd.h>
#include <pci/pci.h>

typedef struct PCIData
{
    struct pci_access* access;
    FF_LIBRARY_SYMBOL(pci_fill_info)
    FF_LIBRARY_SYMBOL(pci_read_byte)
    FF_LIBRARY_SYMBOL(pci_lookup_name)
    FF_LIBRARY_SYMBOL(pci_get_param)

    #if PCI_LIB_VERSION >= 0x030800
        FF_LIBRARY_SYMBOL(pci_get_string_property)
    #endif
} PCIData;

static void pciDetectVendorName(FFGPUResult* gpu, PCIData* pci, struct pci_dev* device)
{
    ffStrbufAppendS(&gpu->vendor, ffGetGPUVendorString(device->vendor_id));

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

static void drmDetectDeviceName(const FFinstance* instance, FFGPUResult* gpu, PCIData* pci, struct pci_dev* device)
{
    u8 revId;
    bool revIdSet = false;

    #if PCI_LIB_VERSION >= 0x030800
        revIdSet = pci->ffpci_fill_info(device, PCI_FILL_CLASS_EXT) & PCI_FILL_CLASS_EXT;
        if(revIdSet)
            revId = device->rev_id;
    #endif

    if(!revIdSet)
    {
        #ifdef __FreeBSD__
            return;
        #else
            revId = pci->ffpci_read_byte(device, PCI_REVISION_ID);
        #endif
    }

    FFstrbuf query;
    ffStrbufInit(&query);
    ffStrbufAppendF(&query, "%X, %X,", device->device_id, revId);

    ffParsePropFileData(instance, "libdrm/amdgpu.ids", query.chars, &gpu->name);

    ffStrbufDestroy(&query);

    const char* removeStrings[] = {
        "AMD ", "ATI ",
        " (TM)", "(TM)",
        " Graphics Adapter", " Graphics", " Series", " Edition"
    };
    ffStrbufRemoveStringsA(&gpu->name, sizeof(removeStrings) / sizeof(removeStrings[0]), removeStrings);
}

static void pciDetectDeviceName(const FFinstance* instance, FFGPUResult* gpu, PCIData* pci, struct pci_dev* device)
{
    if(ffStrbufCompS(&gpu->vendor, FF_GPU_VENDOR_NAME_AMD) == 0)
    {
        drmDetectDeviceName(instance, gpu, pci, device);
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
    #if PCI_LIB_VERSION >= 0x030800
        pci->ffpci_fill_info(device, PCI_FILL_DRIVER);
        ffStrbufAppendS(&gpu->driver, pci->ffpci_get_string_property(device, PCI_FILL_DRIVER));

        if(gpu->driver.length > 0)
            return;
    #endif

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

        //The kernel exposes the device class multiplied by 256 for some reason
        if(tempValue->deviceClass == device->device_class * 256)
        {
            gpu->temperature = tempValue->value;
            return;
        }
    }
}

static void detectType(FFGPUResult* gpu, const PCIData* pci, struct pci_dev* device)
{
    //There is no straightforward way to detect the type of a GPU.
    //The approach taken here is to look at the memory sizes of the device.
    //Since integrated GPUs usually use the system ram, they don't have expansive ROMs
    //and their memory sizes are usually smaller than 1GB.

    if(!(pci->ffpci_fill_info(device, PCI_FILL_SIZES) & PCI_FILL_SIZES))
    {
        gpu->type = FF_GPU_TYPE_UNKNOWN;
        return;
    }

    if(device->rom_size > 0)
    {
        gpu->type = FF_GPU_TYPE_DISCRETE;
        return;
    }

    uint32_t numSizes = sizeof(device->size) / sizeof(device->size[0]);
    for(uint32_t i = 0; i < numSizes; i++)
    {
        if(device->size[i] >= 1024 * 1024 * 1024) //1GB
        {
            gpu->type = FF_GPU_TYPE_DISCRETE;
            return;
        }
    }

    gpu->type = FF_GPU_TYPE_INTEGRATED;
}

static void pciHandleDevice(const FFinstance* instance, FFlist* results, PCIData* pci, struct pci_dev* device)
{
    pci->ffpci_fill_info(device, PCI_FILL_CLASS);

    char class[1024];
    pci->ffpci_lookup_name(pci->access, class, sizeof(class) - 1, PCI_LOOKUP_CLASS, device->device_class);

    if(
        //https://pci-ids.ucw.cz/read/PD/03
        strcasecmp("VGA compatible controller", class) != 0 &&
        strcasecmp("XGA compatible controller", class) != 0 &&
        strcasecmp("3D controller", class)             != 0 &&
        strcasecmp("Display controller", class)        != 0
    ) return;

    pci->ffpci_fill_info(device, PCI_FILL_IDENT);

    FFGPUResult* gpu = ffListAdd(results);

    gpu->id = 0;
        gpu->dedicated.total = gpu->dedicated.used = gpu->shared.total = gpu->shared.used = FF_GPU_VMEM_SIZE_UNSET;

    ffStrbufInit(&gpu->vendor);
    pciDetectVendorName(gpu, pci, device);

    ffStrbufInit(&gpu->name);
    pciDetectDeviceName(instance, gpu, pci, device);

    ffStrbufInit(&gpu->driver);
    pciDetectDriverName(gpu, pci, device);

    detectType(gpu, pci, device);

    gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;

    gpu->temperature = FF_GPU_TEMP_UNSET;
    if(instance->config.gpuTemp)
        pciDetectTemperatur(gpu, device);
}

static const char* pciDetectGPUs(const FFinstance* instance, FFlist* gpus)
{
    PCIData pci;

    FF_LIBRARY_LOAD(libpci, &instance->config.libPCI, "dlopen libpci.so failed", "libpci" FF_LIBRARY_EXTENSION, 4);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libpci, pci_alloc);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libpci, pci_init);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libpci, pci_scan_bus);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libpci, pci_cleanup);

    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libpci, pci, pci_fill_info);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libpci, pci, pci_lookup_name);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libpci, pci, pci_read_byte);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libpci, pci, pci_get_param);

    #if PCI_LIB_VERSION >= 0x030800
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libpci, pci, pci_get_string_property);
    #endif

    pci.access = ffpci_alloc();
    ffpci_init(pci.access);
    ffpci_scan_bus(pci.access);

    struct pci_dev* device = pci.access->devices;
    while(device != NULL)
    {
        pciHandleDevice(instance, gpus, &pci, device);
        device = device->next;
    }

    ffpci_cleanup(pci.access);
    dlclose(libpci);
    return NULL;
}

#endif

const char* ffDetectGPUImpl(FFlist* gpus, const FFinstance* instance)
{
    #ifdef FF_HAVE_LIBPCI
        return pciDetectGPUs(instance, gpus);
    #else
        FF_UNUSED(gpus, instance);
        return "fastfetch is built without libpci support";
    #endif
}

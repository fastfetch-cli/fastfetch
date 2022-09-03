#include "fastfetch.h"
#include "common/printing.h"
#include "common/caching.h"
#include "common/parsing.h"
#include "common/properties.h"
#include "detection/temps.h"
#include "detection/vulkan.h"

#include <stdlib.h>

#define FF_GPU_MODULE_NAME "GPU"
#define FF_GPU_NUM_FORMAT_ARGS 4

#define FF_PCI_VENDOR_NAME_AMD "AMD"
#define FF_PCI_VENDOR_NAME_INTEL "Intel"
#define FF_PCI_VENDOR_NAME_NVIDIA "NVIDIA"

static void printGPUResult(FFinstance* instance, uint8_t index, FFcache* cache, FFGPUResult* result)
{
    FFstrbuf gpu;
    ffStrbufInitA(&gpu, result->vendor.length + 1 + result->device.length);

    if(result->vendor.length > 0)
    {
        ffStrbufAppend(&gpu, &result->vendor);
        ffStrbufAppendC(&gpu, ' ');
    }

    ffStrbufAppend(&gpu, &result->device);

    ffPrintAndAppendToCache(instance, FF_GPU_MODULE_NAME, index, &instance->config.gpu, cache, &gpu, FF_GPU_NUM_FORMAT_ARGS, (FFformatarg[]){
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->vendor},
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->device},
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->driver},
        {FF_FORMAT_ARG_TYPE_DOUBLE, &result->temperature}
    });

    ffStrbufDestroy(&gpu);
}

static void printGPUList(FFinstance* instance, const FFlist* list)
{
    FFcache cache;
    ffCacheOpenWrite(instance, FF_GPU_MODULE_NAME, &cache);

    for(uint8_t i = 0; i < (uint8_t) list->length; i++)
        printGPUResult(instance, list->length == 1 ? 0 : (uint8_t) (i + 1), &cache, ffListGet(list, i));

    ffCacheClose(&cache);
}

#ifdef FF_HAVE_LIBPCI
#include "common/library.h"
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
        ffStrbufAppendS(&gpu->vendor, FF_PCI_VENDOR_NAME_AMD);
    else if(pciIDsContains(device->vendor_id, (const u16[]) {0x03e7, 0x8086, 0x8087, 0}))
        ffStrbufAppendS(&gpu->vendor, FF_PCI_VENDOR_NAME_INTEL);
    else if(pciIDsContains(device->vendor_id, (const u16[]) {0x0955, 0x10de, 0x12d2, 0}))
        ffStrbufAppendS(&gpu->vendor, FF_PCI_VENDOR_NAME_NVIDIA);

    if(gpu->vendor.length > 0)
        return;

    pci->ffpci_lookup_name(pci->access, gpu->vendor.chars, (int) ffStrbufGetFree(&gpu->vendor), PCI_LOOKUP_VENDOR, device->vendor_id);
    ffStrbufRecalculateLength(&gpu->vendor);

    if(ffStrbufFirstIndexS(&gpu->vendor, "AMD") < gpu->vendor.length || ffStrbufFirstIndexS(&gpu->vendor, "ATI") < gpu->vendor.length)
        ffStrbufSetS(&gpu->vendor, FF_PCI_VENDOR_NAME_AMD);
    else if(ffStrbufFirstIndexS(&gpu->vendor, "Intel") < gpu->vendor.length)
        ffStrbufSetS(&gpu->vendor, FF_PCI_VENDOR_NAME_INTEL);
    else if(ffStrbufFirstIndexS(&gpu->vendor, "NVIDIA") < gpu->vendor.length)
        ffStrbufSetS(&gpu->vendor, FF_PCI_VENDOR_NAME_NVIDIA);
}

static void drmDetectDeviceName(FFGPUResult* gpu, PCIData* pci, struct pci_dev* device)
{
    FFstrbuf query;
    ffStrbufInit(&query);
    ffStrbufAppendF(&query, "%X, %X,", device->device_id, pci->ffpci_read_byte(device, PCI_REVISION_ID));

    ffParsePropFile(FASTFETCH_TARGET_DIR_USR"/share/libdrm/amdgpu.ids", query.chars, &gpu->device);

    ffStrbufDestroy(&query);

    const char* removeStrings[] = {
        "AMD ", "ATI ",
        " (TM)", "(TM)",
        " Graphics Adapter", " Graphics", " Series", " Edition"
    };
    ffStrbufRemoveStringsA(&gpu->device, sizeof(removeStrings) / sizeof(removeStrings[0]), removeStrings);
}

static void pciDetectDeviceName(FFGPUResult* gpu, PCIData* pci, struct pci_dev* device)
{
    if(ffStrbufCompS(&gpu->vendor, FF_PCI_VENDOR_NAME_AMD) == 0)
    {
        drmDetectDeviceName(gpu, pci, device);
        if(gpu->device.length > 0)
            return;
    }

    pci->ffpci_lookup_name(pci->access, gpu->device.chars, (int) ffStrbufGetFree(&gpu->device), PCI_LOOKUP_DEVICE, device->vendor_id, device->device_id);
    ffStrbufRecalculateLength(&gpu->device);
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

    ffStrbufInit(&gpu->device);
    pciDetectDeviceName(gpu, pci, device);

    ffStrbufInit(&gpu->driver);
    pciDetectDriverName(gpu, pci, device);

    gpu->temperature = FF_GPU_TEMP_UNSET;
    pciDetectTemperatur(gpu, device);
}

static bool pciPrintGPUs(FFinstance* instance)
{
    PCIData pci;

    FF_LIBRARY_LOAD(libpci, instance->config.libPCI, false, "libpci.so", 4)
    FF_LIBRARY_LOAD_SYMBOL(libpci, pci_alloc, false)
    FF_LIBRARY_LOAD_SYMBOL(libpci, pci_init, false)
    FF_LIBRARY_LOAD_SYMBOL(libpci, pci_scan_bus, false)
    FF_LIBRARY_LOAD_SYMBOL(libpci, pci_cleanup, false)

    FF_LIBRARY_LOAD_SYMBOL_ADRESS(libpci, pci.ffpci_read_byte, pci_read_byte, false)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(libpci, pci.ffpci_read_word, pci_read_word, false)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(libpci, pci.ffpci_lookup_name, pci_lookup_name, false)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(libpci, pci.ffpci_get_param, pci_get_param, false)

    FFlist results;
    ffListInit(&results, sizeof(FFGPUResult));

    pci.access = ffpci_alloc();
    ffpci_init(pci.access);
    ffpci_scan_bus(pci.access);

    struct pci_dev* device = pci.access->devices;
    while(device != NULL)
    {
        pciHandleDevice(&results, &pci, device);
        device = device->next;
    }

    ffpci_cleanup(pci.access);
    dlclose(libpci);

    printGPUList(instance, &results);

    for(uint32_t i = 0; i < results.length; i++)
    {
        FFGPUResult* gpu = ffListGet(&results, i);
        ffStrbufDestroy(&gpu->vendor);
        ffStrbufDestroy(&gpu->device);
        ffStrbufDestroy(&gpu->driver);
    }

    ffListDestroy(&results);
    return results.length > 0;
}

#endif

static bool vulkanPrintGPUs(FFinstance* instance)
{
    const FFVulkanResult* result = ffDetectVulkan(instance);
    if(result->devices.length == 0)
        return false;

    printGPUList(instance, &result->devices);
    return true;
}

void ffPrintGPU(FFinstance* instance)
{
    if(ffPrintFromCache(instance, FF_GPU_MODULE_NAME, &instance->config.gpu, FF_GPU_NUM_FORMAT_ARGS))
        return;

    if(
        getenv("WSLENV") != NULL ||
        getenv("WSL_DISTRO") != NULL ||
        getenv("WSL_INTEROP") != NULL
    ) {
        ffPrintError(instance, FF_GPU_MODULE_NAME, 0, &instance->config.gpu, "WSL doesn't expose senseful GPU names");
        return;
    }

    #ifdef FF_HAVE_LIBPCI
        if(pciPrintGPUs(instance))
            return;
    #endif

    if(vulkanPrintGPUs(instance))
        return;

    ffPrintError(instance, FF_GPU_MODULE_NAME, 0, &instance->config.gpu, "No GPUs found.");
}

#include "fastfetch.h"
#include "common/printing.h"
#include "common/caching.h"
#include "common/parsing.h"
#include "detection/temps.h"
#include "detection/vulkan.h"

#include <stdlib.h>

#define FF_GPU_MODULE_NAME "GPU"
#define FF_GPU_NUM_FORMAT_ARGS 6

static void printGPUResult(FFinstance* instance, uint8_t index, FFcache* cache, FFGPUResult* result)
{
    const char* vendorPretty;
    if(ffStrbufIgnCaseCompS(&result->vendor, "Advanced Micro Devices, Inc. [AMD/ATI]") == 0)
        vendorPretty = "AMD ATI";
    else if(ffStrbufIgnCaseCompS(&result->vendor, "NVIDIA Corporation") == 0)
        vendorPretty = "Nvidia";
    else if(ffStrbufIgnCaseCompS(&result->vendor, "Intel Corporation") == 0)
        vendorPretty = "Intel";
    else
        vendorPretty = result->vendor.chars;

    FFstrbuf namePretty;
    ffStrbufInitCopy(&namePretty, &result->name);
    ffStrbufSubstrBeforeLastC(&namePretty, ']');
    ffStrbufSubstrAfterFirstC(&namePretty, '[');

    FFstrbuf gpu;
    ffStrbufInitA(&gpu, result->vendor.length + 1 + namePretty.length);

    if(ffStrSet(vendorPretty))
    {
        ffStrbufAppendS(&gpu, vendorPretty);
        ffStrbufAppendC(&gpu, ' ');
    }

    ffStrbufAppend(&gpu, &namePretty);

    ffPrintAndAppendToCache(instance, FF_GPU_MODULE_NAME, index, &instance->config.gpu, cache, &gpu, FF_GPU_NUM_FORMAT_ARGS, (FFformatarg[]){
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->vendor},
        {FF_FORMAT_ARG_TYPE_STRING, vendorPretty},
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->name},
        {FF_FORMAT_ARG_TYPE_STRBUF, &namePretty},
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->driver},
        {FF_FORMAT_ARG_TYPE_DOUBLE, &result->temperature}
    });

    ffStrbufDestroy(&result->vendor);
    ffStrbufDestroy(&result->name);
    ffStrbufDestroy(&result->driver);
    ffStrbufDestroy(&gpu);
    ffStrbufDestroy(&namePretty);
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

//see https://github.com/pciutils/pciutils/blob/5bdf63b6b1bc35b59c4b3f47f7ca83ca1868155b/ls-kernel.c#L220
static void pciGetDriver(struct pci_dev* dev, FFstrbuf* driver, char*(*ffpci_get_param)(struct pci_access*, char*))
{
    if(dev->access->method != PCI_ACCESS_SYS_BUS_PCI)
        return;

    const char* base = ffpci_get_param(dev->access, "sysfs.path");
    if(!ffStrSet(base))
        return;

    FFstrbuf path;
    ffStrbufInitA(&path, 64);
    ffStrbufAppendF(&path, "%s/devices/%04x:%02x:%02x.%d/driver", base, dev->domain, dev->bus, dev->dev, dev->func);

    ffStrbufEnsureFree(driver, 1023);
    ssize_t resultLength = readlink(path.chars, driver->chars, driver->allocated - 1); //-1 for null terminator
    if(resultLength > 0)
    {
        driver->length = (uint32_t) resultLength;
        driver->chars[resultLength] = '\0';
        ffStrbufSubstrAfterLastC(driver, '/');
    }

    ffStrbufDestroy(&path);
}

static double pciGetTemperatur(const FFinstance* instance, uint16_t deviceClass)
{
    const FFTempsResult* tempsResult = ffDetectTemps(instance);

    for(uint32_t i = 0; i < tempsResult->values.length; i++)
    {
        FFTempValue* tempValue = ffListGet(&tempsResult->values, i);

        uint32_t tempClass;
        if(sscanf(tempValue->deviceClass.chars, "%x", &tempClass) != 1)
            continue;

        //The kernel exposes the device class multiplied by 256 for some reason
        if(tempClass == deviceClass * 256)
            return tempValue->value;
    }

    return 0.0 / 0.0; //NaN
}

static bool pciPrintGPUs(FFinstance* instance)
{
    FF_LIBRARY_LOAD(pci, instance->config.libPCI, false, "libpci.so", 4)
    FF_LIBRARY_LOAD_SYMBOL(pci, pci_alloc, false)
    FF_LIBRARY_LOAD_SYMBOL(pci, pci_init, false)
    FF_LIBRARY_LOAD_SYMBOL(pci, pci_scan_bus, false)
    FF_LIBRARY_LOAD_SYMBOL(pci, pci_fill_info, false)
    FF_LIBRARY_LOAD_SYMBOL(pci, pci_lookup_name, false)
    FF_LIBRARY_LOAD_SYMBOL(pci, pci_get_param, false)
    FF_LIBRARY_LOAD_SYMBOL(pci, pci_cleanup, false)

    FFlist results;
    ffListInit(&results, sizeof(FFGPUResult));

    struct pci_access *pacc = ffpci_alloc();
    ffpci_init(pacc);
    ffpci_scan_bus(pacc);

    struct pci_dev* dev;
    for (dev=pacc->devices; dev; dev=dev->next)
    {
        ffpci_fill_info(dev, PCI_FILL_IDENT | PCI_FILL_CLASS);
        char class[1024];
        ffpci_lookup_name(pacc, class, sizeof(class), PCI_LOOKUP_CLASS, dev->device_class);
        if(
            strcasecmp("VGA compatible controller", class) == 0 ||
            strcasecmp("3D controller", class)             == 0 ||
            strcasecmp("Display controller", class)        == 0
        ) {
            FFGPUResult* result = ffListAdd(&results);

            ffStrbufInitA(&result->vendor, 256);
            ffpci_lookup_name(pacc, result->vendor.chars, (int) ffStrbufGetFree(&result->vendor), PCI_LOOKUP_VENDOR, dev->vendor_id, dev->device_id);
            ffStrbufRecalculateLength(&result->vendor);

            ffStrbufInitA(&result->name, 256);
            ffpci_lookup_name(pacc, result->name.chars, (int) ffStrbufGetFree(&result->name), PCI_LOOKUP_DEVICE, dev->vendor_id, dev->device_id);
            ffStrbufRecalculateLength(&result->name);

            ffStrbufInit(&result->driver);
            result->temperature = 0;

            //We only need it for the format string, so don't detect it if it isn't needed
            if(instance->config.gpu.outputFormat.length > 0)
            {
                pciGetDriver(dev, &result->driver, ffpci_get_param);
                result->temperature = pciGetTemperatur(instance, dev->device_class);
            }
        };
    }

    ffpci_cleanup(pacc);
    dlclose(pci);

    if(results.length == 0)
    {
        ffListDestroy(&results);
        return false;
    }

    printGPUList(instance, &results);

    ffListDestroy(&results);
    return true;
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

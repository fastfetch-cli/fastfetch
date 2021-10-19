#include "fastfetch.h"

#include <string.h>

#define FF_GPU_MODULE_NAME "GPU"
#define FF_GPU_NUM_FORMAT_ARGS 4

#ifndef FF_HAVE_LIBPCI

void ffPrintGPU(FFinstance* instance)
{
    ffPrintError(instance, FF_GPU_MODULE_NAME, 0, &instance->config.gpuKey, &instance->config.gpuFormat, FF_GPU_NUM_FORMAT_ARGS, "Fastfetch was compiled without libpci support");
}

#else

#include <dlfcn.h>
#include <pci/pci.h>

static void printGPU(FFinstance* instance, struct pci_access* pacc, struct pci_dev* dev, FFcache* cache, uint8_t counter, char*(*ffpci_lookup_name)(struct pci_access*, char*, int, int, ...))
{
    char vendor[512];
    vendor[0] = '\0';
    ffpci_lookup_name(pacc, vendor, sizeof(vendor), PCI_LOOKUP_VENDOR, dev->vendor_id, dev->device_id);

    const char* vendorPretty;
    if(strcasecmp(vendor, "Advanced Micro Devices, Inc. [AMD/ATI]") == 0)
        vendorPretty = "AMD ATI";
    else if(strcasecmp(vendor, "NVIDIA Corporation") == 0)
        vendorPretty = "Nvidia";
    else if(strcasecmp(vendor, "Intel Corporation") == 0)
        vendorPretty = "Intel";
    else
        vendorPretty = vendor;

    char name[512];
    name[0] = '\0';
    ffpci_lookup_name(pacc, name, sizeof(name), PCI_LOOKUP_DEVICE, dev->vendor_id, dev->device_id);

    FFstrbuf namePretty;
    ffStrbufInitA(&namePretty, 512);
    ffStrbufAppendS(&namePretty, name);
    ffStrbufSubstrBeforeLastC(&namePretty, ']');
    ffStrbufSubstrAfterFirstC(&namePretty, '[');

    FFstrbuf gpu;
    ffStrbufInitA(&gpu, 128);

    ffStrbufSetF(&gpu, "%s %s", vendorPretty, namePretty.chars);

    ffPrintAndAppendToCache(instance, FF_GPU_MODULE_NAME, counter, &instance->config.gpuKey, cache, &gpu, &instance->config.gpuFormat, FF_GPU_NUM_FORMAT_ARGS, (FFformatarg[]){
        {FF_FORMAT_ARG_TYPE_STRING, vendor},
        {FF_FORMAT_ARG_TYPE_STRING, vendorPretty},
        {FF_FORMAT_ARG_TYPE_STRING, name},
        {FF_FORMAT_ARG_TYPE_STRBUF, &namePretty}
    });

    ffStrbufDestroy(&gpu);
    ffStrbufDestroy(&namePretty);
}

void ffPrintGPU(FFinstance* instance)
{
    if(ffPrintFromCache(instance, FF_GPU_MODULE_NAME, &instance->config.gpuKey, &instance->config.gpuFormat, FF_GPU_NUM_FORMAT_ARGS))
        return;

    const char* pciLibName = instance->config.libPCI.length == 0 ? "libpci.so" : instance->config.libPCI.chars;
    void* pci = dlopen(pciLibName, RTLD_LAZY);
    if(pci == NULL)
    {
        ffPrintError(instance, FF_GPU_MODULE_NAME, 0, &instance->config.gpuKey, &instance->config.gpuFormat, FF_GPU_NUM_FORMAT_ARGS, "dlopen(\"%s\", RTLD_LAZY) == NULL", pciLibName);
        return;
    }

    struct pci_access*(*ffpci_alloc)() = dlsym(pci, "pci_alloc");
    if(ffpci_alloc == NULL)
    {
        dlclose(pci);
        ffPrintError(instance, FF_GPU_MODULE_NAME, 0, &instance->config.gpuKey, &instance->config.gpuFormat, FF_GPU_NUM_FORMAT_ARGS, "dlsym(pci, \"pci_alloc\") == NULL");
        return;
    }

    void(*ffpci_init)(struct pci_access*) = dlsym(pci, "pci_init");
    if(ffpci_init == NULL)
    {
        dlclose(pci);
        ffPrintError(instance, FF_GPU_MODULE_NAME, 0, &instance->config.gpuKey, &instance->config.gpuFormat, FF_GPU_NUM_FORMAT_ARGS, "dlsym(pci, \"pci_init\") == NULL");
        return;
    }

    void(*ffpci_scan_bus)(struct pci_access*) = dlsym(pci, "pci_scan_bus");
    if(ffpci_scan_bus == NULL)
    {
        dlclose(pci);
        ffPrintError(instance, FF_GPU_MODULE_NAME, 0, &instance->config.gpuKey, &instance->config.gpuFormat, FF_GPU_NUM_FORMAT_ARGS, "dlsym(pci, \"pci_init\") == NULL");
        return;
    }

    int(*ffpci_fill_info)(struct pci_dev*, int) = dlsym(pci, "pci_fill_info");
    if(ffpci_fill_info == NULL)
    {
        dlclose(pci);
        ffPrintError(instance, FF_GPU_MODULE_NAME, 0, &instance->config.gpuKey, &instance->config.gpuFormat, FF_GPU_NUM_FORMAT_ARGS, "dlsym(pci, \"pci_fill_info\") == NULL");
        return;
    }

    char*(*ffpci_lookup_name)(struct pci_access*, char*, int, int, ...) = dlsym(pci, "pci_lookup_name");
    if(ffpci_lookup_name == NULL)
    {
        dlclose(pci);
        ffPrintError(instance, FF_GPU_MODULE_NAME, 0, &instance->config.gpuKey, &instance->config.gpuFormat, FF_GPU_NUM_FORMAT_ARGS, "dlsym(pci, \"pci_lookup_name\") == NULL");
        return;
    }

    void(*ffpci_cleanup)(struct pci_access*) = dlsym(pci, "pci_cleanup");
    if(ffpci_cleanup == NULL)
    {
        dlclose(pci);
        ffPrintError(instance, FF_GPU_MODULE_NAME, 0, &instance->config.gpuKey, &instance->config.gpuFormat, FF_GPU_NUM_FORMAT_ARGS, "dlsym(pci, \"pci_cleanup\") == NULL");
        return;
    }

    struct pci_access *pacc = ffpci_alloc();
    ffpci_init(pacc);
    ffpci_scan_bus(pacc);

    FFlist devices;
    ffListInitA(&devices, sizeof(struct pci_dev*), 4);

    FFcache cache;
    ffCacheOpenWrite(instance, FF_GPU_MODULE_NAME, &cache);

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
        ) *(struct pci_dev**)ffListAdd(&devices) = dev;
    }

    for(uint32_t i = 0; i < devices.length; i++)
        printGPU(instance, pacc, *(struct pci_dev**)ffListGet(&devices, i), &cache, devices.length == 1 ? 0 : i + 1, ffpci_lookup_name);

    if(devices.length == 0)
        ffPrintError(instance, FF_GPU_MODULE_NAME, 0, &instance->config.gpuKey, &instance->config.gpuFormat, FF_GPU_NUM_FORMAT_ARGS, "No GPU found");

    ffCacheClose(&cache);
    ffListDestroy(&devices);
    ffpci_cleanup(pacc);
    dlclose(pci);
}

#endif

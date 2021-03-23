#include "fastfetch.h"

#include <pci/pci.h>
#include <dlfcn.h>

static void handleGPU(FFinstance* instance, struct pci_access* pacc, struct pci_dev* dev, uint16_t counter, char*(*ffpci_lookup_name)(struct pci_access*, char*, int, int, ...))
{   
    char key[8];
    sprintf(key, "GPU%hu", counter);

    if(ffPrintCachedValue(instance, key))
        return;

    char vendor[512];
    ffpci_lookup_name(pacc, vendor, sizeof(vendor), PCI_LOOKUP_VENDOR, dev->vendor_id, dev->device_id);

    char vendorPretty[512];
    if(strcasecmp(vendor, "Advanced Micro Devices, Inc. [AMD/ATI]") == 0)
        strcpy(vendorPretty, "AMD ATI");
    else
        strcpy(vendorPretty, vendor);

    char name[512];
    ffpci_lookup_name(pacc, name, sizeof(name), PCI_LOOKUP_DEVICE, dev->vendor_id, dev->device_id);

    FF_STRBUF_CREATE(gpu);

    if(instance->config.gpuFormat.length == 0)
    {
        ffStrbufSetF(&gpu, "%s %s", vendorPretty, name);
    }
    else
    {
        ffParseFormatString(&gpu, &instance->config.gpuFormat, 2, 
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, vendorPretty},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, name}
        );
    }

    ffPrintAndSaveCachedValue(instance, key, &gpu);
    ffStrbufDestroy(&gpu);
}

void ffPrintGPU(FFinstance* instance)
{
    void* pci;
    if(instance->config.libPCI.length == 0)
        pci = dlopen("libpci.so", RTLD_LAZY);
    else
        pci = dlopen(instance->config.libPCI.chars, RTLD_LAZY);
    if(pci == NULL)
    {
        ffPrintError(instance, "GPU", "dlopen(\"libpci.so\", RTLD_LAZY) == NULL");
        return;
    }

    struct pci_access*(*ffpci_alloc)() = dlsym(pci, "pci_alloc");
    if(ffpci_alloc == NULL)
    {
        ffPrintError(instance, "GPU", "dlsym(pci, \"pci_alloc\") == NULL");
        return;
    }

    void(*ffpci_init)(struct pci_access*) = dlsym(pci, "pci_init");
    if(ffpci_init == NULL)
    {
        ffPrintError(instance, "GPU", "dlsym(pci, \"pci_init\") == NULL");
        return;
    }

    void(*ffpci_scan_bus)(struct pci_access*) = dlsym(pci, "pci_scan_bus");
    if(ffpci_scan_bus == NULL)
    {
        ffPrintError(instance, "GPU", "dlsym(pci, \"pci_init\") == NULL");
        return;
    }

    int(*ffpci_fill_info)(struct pci_dev*, int) = dlsym(pci, "pci_fill_info");
    if(ffpci_fill_info == NULL)
    {
        ffPrintError(instance, "GPU", "dlsym(pci, \"pci_fill_info\") == NULL");
        return;
    }

    char*(*ffpci_lookup_name)(struct pci_access*, char*, int, int, ...) = dlsym(pci, "pci_lookup_name");
    if(ffpci_lookup_name == NULL)
    {
        ffPrintError(instance, "GPU", "dlsym(pci, \"pci_lookup_name\") == NULL");
        return;
    }

    void(*ffpci_cleanup)(struct pci_access*) = dlsym(pci, "pci_cleanup");
    if(ffpci_cleanup == NULL)
    {
        ffPrintError(instance, "GPU", "dlsym(pci, \"pci_cleanup\") == NULL");
        return;
    }

    uint16_t counter = 0;

    struct pci_access *pacc;
    struct pci_dev *dev;

    pacc = ffpci_alloc();
    ffpci_init(pacc);
    ffpci_scan_bus(pacc);
    for (dev=pacc->devices; dev; dev=dev->next)
    {
        ffpci_fill_info(dev, PCI_FILL_IDENT | PCI_FILL_CLASS);
        char class[1024];
        ffpci_lookup_name(pacc, class, sizeof(class), PCI_LOOKUP_CLASS, dev->device_class);
        if(
            strcasecmp("VGA compatible controller", class) == 0 ||
            strcasecmp("3D controller", class)             == 0 ||
            strcasecmp("Display controller", class)        == 0 )
        {
            handleGPU(instance, pacc, dev, counter++, ffpci_lookup_name);
        }
    }
    ffpci_cleanup(pacc);

    dlclose(pci);
}
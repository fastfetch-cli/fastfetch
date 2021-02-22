#include "fastfetch.h"

#include <string.h>
#include <pci/pci.h>

static void handleGPU(FFstate* state, struct pci_access* pacc, struct pci_dev* dev, uint16_t counter)
{   
    char key[8];
    sprintf(key, "GPU%hu", counter);

    if(ffPrintCachedValue(state, key))
        return;

    char gpu[1024];

    char vendor[512];
    pci_lookup_name(pacc, vendor, sizeof(vendor), PCI_LOOKUP_VENDOR, dev->vendor_id, dev->device_id);

    char name[512];
    pci_lookup_name(pacc, name, sizeof(name), PCI_LOOKUP_DEVICE, dev->vendor_id, dev->device_id);

    if(strcmp(vendor, "Advanced Micro Devices, Inc. [AMD/ATI]") == 0)
    {
        strcpy(gpu, "AMD ATI ");
    }
    else
    {
        strcpy(gpu, vendor);
        strcat(gpu, " ");
    }

    strcat(gpu, name);

    ffPrintAndSaveCachedValue(state, key, gpu);
}

void ffPrintGPU(FFstate* state)
{
    uint16_t counter = 0;

    struct pci_access *pacc;
    struct pci_dev *dev;

    pacc = pci_alloc();
    pci_init(pacc);
    pci_scan_bus(pacc);
    for (dev=pacc->devices; dev; dev=dev->next)
    {
        pci_fill_info(dev, PCI_FILL_IDENT | PCI_FILL_CLASS);
        char class[1024];
        pci_lookup_name(pacc, class, sizeof(class), PCI_LOOKUP_CLASS, dev->device_class);
        if(
            strcmp("VGA compatible controller", class) == 0 ||
            strcmp("3D controller", class)             == 0 ||
            strcmp("Display controller", class)        == 0 )
        {
            handleGPU(state, pacc, dev, counter++);
        }
    }
    pci_cleanup(pacc);
}
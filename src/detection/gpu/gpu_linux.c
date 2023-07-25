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
#include <setjmp.h>

// Fix building on Ubuntu 20.04
#ifndef PCI_IORESOURCE_MEM
    #define PCI_IORESOURCE_MEM 0x00000200
#endif
#ifndef PCI_IORESOURCE_PREFETCH
    #define PCI_IORESOURCE_PREFETCH 0x00002000
#endif

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

    if(ffStrbufContainS(&gpu->vendor, "AMD") || ffStrbufContainS(&gpu->vendor, "ATI"))
        ffStrbufSetS(&gpu->vendor, FF_GPU_VENDOR_NAME_AMD);
    else if(ffStrbufContainS(&gpu->vendor, "Intel"))
        ffStrbufSetS(&gpu->vendor, FF_GPU_VENDOR_NAME_INTEL);
    else if(ffStrbufContainS(&gpu->vendor, "NVIDIA"))
        ffStrbufSetS(&gpu->vendor, FF_GPU_VENDOR_NAME_NVIDIA);
}

static void drmDetectDeviceName(FFGPUResult* gpu, PCIData* pci, struct pci_dev* device)
{
    u8 revId = 0;
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

    FF_STRBUF_AUTO_DESTROY query = ffStrbufCreateF("%X, %X,", device->device_id, revId);
    ffParsePropFileData("libdrm/amdgpu.ids", query.chars, &gpu->name);

    const char* removeStrings[] = {
        "AMD ", "ATI ",
        " (TM)", "(TM)",
        " Graphics Adapter", " Graphics", " Series", " Edition"
    };
    ffStrbufRemoveStrings(&gpu->name, sizeof(removeStrings) / sizeof(removeStrings[0]), removeStrings);
}

static void pciDetectDeviceName(FFGPUResult* gpu, PCIData* pci, struct pci_dev* device)
{
    if(ffStrbufEqualS(&gpu->vendor, FF_GPU_VENDOR_NAME_AMD))
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
    #if PCI_LIB_VERSION >= 0x030800
        pci->ffpci_fill_info(device, PCI_FILL_DRIVER);
        ffStrbufAppendS(&gpu->driver, pci->ffpci_get_string_property(device, PCI_FILL_DRIVER));

        if(gpu->driver.length > 0)
            return;
    #endif

    const char* base = pci->ffpci_get_param(pci->access, "sysfs.path");
    if(!ffStrSet(base))
        return;

    FF_STRBUF_AUTO_DESTROY path = ffStrbufCreateF("%s/devices/%04x:%02x:%02x.%d/driver", base, device->domain, device->bus, device->dev, device->func);
    ffStrbufEnsureFree(&gpu->driver, 1023);
    ssize_t resultLength = readlink(path.chars, gpu->driver.chars, gpu->driver.allocated - 1); //-1 for null terminator
    if(resultLength > 0)
    {
        gpu->driver.length = (uint32_t) resultLength;
        gpu->driver.chars[resultLength] = '\0';
        ffStrbufSubstrAfterLastC(&gpu->driver, '/');
    }
}

FF_MAYBE_UNUSED static void pciDetectTemp(FFGPUResult* gpu, struct pci_dev* device)
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

FF_MAYBE_UNUSED static bool pciDetectMemory(FFGPUResult* gpu, const PCIData* pci, struct pci_dev* device)
{
    gpu->dedicated.used = gpu->shared.used = FF_GPU_VMEM_SIZE_UNSET;

    uint32_t flags = (uint32_t) pci->ffpci_fill_info(device, PCI_FILL_IO_FLAGS | PCI_FILL_SIZES);
    if (!(flags & PCI_FILL_IO_FLAGS) || !(flags & PCI_FILL_SIZES))
    {
        gpu->dedicated.total = gpu->shared.total = FF_GPU_VMEM_SIZE_UNSET;
        return false;
    }

    gpu->dedicated.total = gpu->shared.total = 0;
    for (uint32_t i = 0; i < sizeof(device->size) / sizeof(device->size[0]); i++)
    {
        if (!(device->flags[i] & PCI_IORESOURCE_MEM)) continue;

        // Assume dedicated memories are prefetchable
        // At least it's true for my laptop
        if (device->flags[i] & PCI_IORESOURCE_PREFETCH)
            gpu->dedicated.total += device->size[i];
        else
            gpu->shared.total += device->size[i];
    }

    if (gpu->dedicated.total == 0 && gpu->shared.total == 0)
    {
        gpu->dedicated.total = gpu->shared.total = FF_GPU_VMEM_SIZE_UNSET;
        return false;
    }

    return true;
}

FF_MAYBE_UNUSED static void pciDetectType(FFGPUResult* gpu)
{
    //There is no straightforward way to detect the type of a GPU.
    //The approach taken here is to look at the memory sizes of the device.
    //Since integrated GPUs usually use the system ram, they don't have expansive ROMs
    //and their memory sizes are usually smaller than 1GB.
    if (gpu->dedicated.total != FF_GPU_VMEM_SIZE_UNSET)
    {
        gpu->type = gpu->dedicated.total > 1024 * 1024 * 1024 // 1GB
            ? FF_GPU_TYPE_DISCRETE
            : FF_GPU_TYPE_INTEGRATED;
    }
    else
        gpu->type = FF_GPU_TYPE_UNKNOWN;
}

static void pciHandleDevice(FF_MAYBE_UNUSED const FFGPUOptions* options, FFlist* results, PCIData* pci, struct pci_dev* device)
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

    ffStrbufInit(&gpu->vendor);
    pciDetectVendorName(gpu, pci, device);

    ffStrbufInit(&gpu->name);
    pciDetectDeviceName(gpu, pci, device);

    ffStrbufInit(&gpu->driver);
    pciDetectDriverName(gpu, pci, device);

    #if FF_USE_PCI_MEMORY
        // Libpci reports at least 2 false results (#495, #497)
        pciDetectMemory(gpu, pci, device);
        pciDetectType(gpu);
    #else
        gpu->dedicated.used = gpu->shared.used = gpu->dedicated.total = gpu->shared.total = FF_GPU_VMEM_SIZE_UNSET;
        gpu->type = FF_GPU_TYPE_UNKNOWN;
    #endif

    gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
    gpu->temperature = FF_GPU_TEMP_UNSET;

    #ifdef __linux__
    if(options->temp)
        pciDetectTemp(gpu, device);
    #endif
}

jmp_buf pciInitJmpBuf;
static void  __attribute__((__noreturn__))
handlePciInitError(FF_MAYBE_UNUSED char *msg, ...)
{
    longjmp(pciInitJmpBuf, 1);
}
// https://github.com/pciutils/pciutils/blob/bca0412843fa650c749128ade03f35ab3e8fe2b9/lib/init.c#L186
static void __attribute__((__noreturn__))
handlePciGenericError(char *msg, ...)
{
    va_list args;

    va_start(args, msg);
    fputs("pcilib: ", stderr);
    vfprintf(stderr, msg, args);
    va_end(args);
    fputc('\n', stderr);
    exit(1);
}
static void handlePciWarning(FF_MAYBE_UNUSED char *msg, ...)
{
    // noop
}

static const char* pciDetectGPUs(const FFGPUOptions* options, FFlist* gpus)
{
    PCIData pci;

    FF_LIBRARY_LOAD(libpci, &instance.config.libPCI, "dlopen libpci.so failed", "libpci" FF_LIBRARY_EXTENSION, 4);
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
    pci.access->warning = handlePciWarning;
    pci.access->error = handlePciInitError;
    if(setjmp(pciInitJmpBuf) == 0) // https://github.com/pciutils/pciutils/issues/136
        ffpci_init(pci.access);
    else
        return "pcilib: Cannot find any working access method.";
    pci.access->error = handlePciGenericError; // set back to generic error so we don't mess up error handling in other places

    ffpci_scan_bus(pci.access);

    struct pci_dev* device = pci.access->devices;
    while(device != NULL)
    {
        pciHandleDevice(options, gpus, &pci, device);
        device = device->next;
    }

    ffpci_cleanup(pci.access);
    return NULL;
}

#endif

const char* ffDetectGPUImpl(const FFGPUOptions* options, FFlist* gpus)
{
    #ifdef FF_HAVE_LIBPCI
        return pciDetectGPUs(options, gpus);
    #else
        FF_UNUSED(options, gpus);
        return "fastfetch is built without libpci support";
    #endif
}

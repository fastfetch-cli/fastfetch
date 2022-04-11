#include "fastfetch.h"

#include <string.h>

#define FF_GPU_MODULE_NAME "GPU"
#define FF_GPU_NUM_FORMAT_ARGS 5

typedef struct GPUResult
{
    FFstrbuf vendor;
    FFstrbuf name;
    FFstrbuf driver;
} GPUResult;

#ifdef FF_HAVE_VULKAN
#include <vulkan/vulkan.h>

static void vulkanFillGPUs(FFinstance* instance, FFlist* results)
{
    FF_LIBRARY_LOAD(vulkan, instance->config.libVulkan, , "libvulkan.so", 2)
    FF_LIBRARY_LOAD_SYMBOL(vulkan, vkCreateInstance,)
    FF_LIBRARY_LOAD_SYMBOL(vulkan, vkDestroyInstance,)
    FF_LIBRARY_LOAD_SYMBOL(vulkan, vkEnumeratePhysicalDevices,)
    FF_LIBRARY_LOAD_SYMBOL(vulkan, vkGetPhysicalDeviceProperties,)

    //Some drivers (nvdc) print messages to stdout
    //and thats the best way i found to disable that
    ffSuppressIO(true);

    const VkApplicationInfo applicationInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = NULL,
        .pApplicationName = FASTFETCH_PROJECT_NAME,
        .applicationVersion = VK_MAKE_VERSION(FASTFETCH_PROJECT_VERSION_MAJOR, 0, 0),
        .pEngineName = "vulkanPrintGPUs",
        .engineVersion = VK_MAKE_VERSION(FASTFETCH_PROJECT_VERSION_MAJOR, 0, 0),
        .apiVersion = VK_API_VERSION_1_0,
    };

    const VkInstanceCreateInfo instanceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = NULL,
        .pApplicationInfo = &applicationInfo,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = NULL,
        .enabledExtensionCount = 0,
        .ppEnabledExtensionNames = NULL,
        .flags = 0
    };

    VkInstance vkInstance;
    if(ffvkCreateInstance(&instanceCreateInfo, NULL, &vkInstance) != VK_SUCCESS)
    {
        dlclose(vulkan);
        ffSuppressIO(false);
        return;
    }

    uint32_t physicalDeviceCount;
    if(ffvkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, NULL) != VK_SUCCESS)
    {
        ffvkDestroyInstance(vkInstance, NULL);
        dlclose(vulkan);
        ffSuppressIO(false);
        return;
    }

    VkPhysicalDevice* physicalDevices = malloc(sizeof(VkPhysicalDevice) * physicalDeviceCount);
    if(ffvkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, physicalDevices) != VK_SUCCESS)
    {
        free(physicalDevices);
        ffvkDestroyInstance(vkInstance, NULL);
        dlclose(vulkan);
        ffSuppressIO(false);
        return;
    }

    for(uint32_t i = 0; i < physicalDeviceCount; i++)
    {
        VkPhysicalDeviceProperties physicalDeviceProperties;
        ffvkGetPhysicalDeviceProperties(physicalDevices[i], &physicalDeviceProperties);

        //We don't want softare rasterizers
        if(physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU)
            continue;

        GPUResult* result = ffListAdd(results);
        ffStrbufInit(&result->vendor);
        ffStrbufInit(&result->name);
        ffStrbufInit(&result->driver);

        ffStrbufAppendS(&result->name, physicalDeviceProperties.deviceName);
    }

    free(physicalDevices);
    ffvkDestroyInstance(vkInstance, NULL);
    dlclose(vulkan);
    ffSuppressIO(false);
    return;
}

#endif

#ifdef FF_HAVE_LIBPCI
#include <pci/pci.h>
#include <unistd.h>

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

static void pciFillGPUs(FFinstance* instance, FFlist* results)
{
    FF_LIBRARY_LOAD(pci, instance->config.libPCI, , "libpci.so", 4)
    FF_LIBRARY_LOAD_SYMBOL(pci, pci_alloc,)
    FF_LIBRARY_LOAD_SYMBOL(pci, pci_init,)
    FF_LIBRARY_LOAD_SYMBOL(pci, pci_scan_bus,)
    FF_LIBRARY_LOAD_SYMBOL(pci, pci_fill_info,)
    FF_LIBRARY_LOAD_SYMBOL(pci, pci_lookup_name,)
    FF_LIBRARY_LOAD_SYMBOL(pci, pci_get_param,)
    FF_LIBRARY_LOAD_SYMBOL(pci, pci_cleanup,)

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
            GPUResult* result = ffListAdd(results);

            ffStrbufInitA(&result->vendor, 256);
            ffpci_lookup_name(pacc, result->vendor.chars, (int) result->vendor.allocated -1, PCI_LOOKUP_VENDOR, dev->vendor_id, dev->device_id);
            ffStrbufRecalculateLength(&result->vendor);

            ffStrbufInitA(&result->name, 256);
            ffpci_lookup_name(pacc, result->name.chars, (int) result->name.allocated - 1, PCI_LOOKUP_DEVICE, dev->vendor_id, dev->device_id);
            ffStrbufRecalculateLength(&result->name);

            ffStrbufInit(&result->driver);
            if(instance->config.gpuFormat.length > 0) //We only need it for the format string, so don't detect it if it isn't needed
                pciGetDriver(dev, &result->driver, ffpci_get_param);
        };
    }

    ffpci_cleanup(pacc);
    dlclose(pci);
}

#endif

static void printGPUResult(FFinstance* instance, uint8_t index, FFcache* cache, GPUResult* result)
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

    ffPrintAndAppendToCache(instance, FF_GPU_MODULE_NAME, index, &instance->config.gpuKey, cache, &gpu, &instance->config.gpuFormat, FF_GPU_NUM_FORMAT_ARGS, (FFformatarg[]){
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->vendor},
        {FF_FORMAT_ARG_TYPE_STRING, vendorPretty},
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->name},
        {FF_FORMAT_ARG_TYPE_STRBUF, &namePretty},
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->driver},
    });

    ffStrbufDestroy(&result->vendor);
    ffStrbufDestroy(&result->name);
    ffStrbufDestroy(&result->driver);
    ffStrbufDestroy(&gpu);
    ffStrbufDestroy(&namePretty);
}

void ffPrintGPU(FFinstance* instance)
{
    if(ffPrintFromCache(instance, FF_GPU_MODULE_NAME, &instance->config.gpuKey, &instance->config.gpuFormat, FF_GPU_NUM_FORMAT_ARGS))
        return;

    FFlist gpus;
    ffListInitA(&gpus, sizeof(GPUResult), 4);

    FFcache cache;
    ffCacheOpenWrite(instance, FF_GPU_MODULE_NAME, &cache);

    #ifdef FF_HAVE_LIBPCI
        pciFillGPUs(instance, &gpus);
    #endif

    #ifdef FF_HAVE_VULKAN
        if(gpus.length == 0)
            vulkanFillGPUs(instance, &gpus);
    #endif

    for(uint8_t i = 0; i < (uint8_t) gpus.length; i++)
        printGPUResult(instance, gpus.length == 1 ? 0 : (uint8_t) (i + 1), &cache, ffListGet(&gpus, i));

    if(gpus.length == 0)
        ffPrintError(instance, FF_GPU_MODULE_NAME, 0, &instance->config.gpuKey, &instance->config.gpuFormat, FF_GPU_NUM_FORMAT_ARGS, "No GPUs found.");

    ffCacheClose(&cache);
    ffListDestroy(&gpus);
}

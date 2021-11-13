#include "fastfetch.h"

#include <string.h>

#define FF_GPU_MODULE_NAME "GPU"
#define FF_GPU_NUM_FORMAT_ARGS 4

typedef struct GPUResult
{
    FFstrbuf vendor;
    FFstrbuf name;
} GPUResult;

#ifdef FF_HAVE_VULKAN
#include <vulkan/vulkan.h>

static void vulkanFillGPUs(FFinstance* instance, FFlist* results)
{
    FF_LIBRARY_LOAD(vulkan, "libvulkan.so", instance->config.libVulkan,)
    FF_LIBRARY_LOAD_SYMBOL(vulkan, vkCreateInstance,)
    FF_LIBRARY_LOAD_SYMBOL(vulkan, vkDestroyInstance,)
    FF_LIBRARY_LOAD_SYMBOL(vulkan, vkEnumeratePhysicalDevices,)
    FF_LIBRARY_LOAD_SYMBOL(vulkan, vkGetPhysicalDeviceProperties,)

    const VkApplicationInfo applicationInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = NULL,
        .pApplicationName = FASTFETCH_PROJECT_NAME,
        .applicationVersion = VK_MAKE_VERSION(FASTFETCH_PROJECT_NAME, 0, 0),
        .pEngineName = "vulkanPrintGPUs",
        .engineVersion = VK_MAKE_VERSION(FASTFETCH_PROJECT_NAME, 0, 0),
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
        return;
    }

    uint32_t physicalDeviceCount;
    if(ffvkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, NULL) != VK_SUCCESS)
    {
        ffvkDestroyInstance(vkInstance, NULL);
        dlclose(vulkan);
        return;
    }

    VkPhysicalDevice* physicalDevices = malloc(sizeof(VkPhysicalDevice) * physicalDeviceCount);
    if(ffvkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, physicalDevices) != VK_SUCCESS)
    {
        free(physicalDevices);
        ffvkDestroyInstance(vkInstance, NULL);
        dlclose(vulkan);
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

        ffStrbufAppendS(&result->name, physicalDeviceProperties.deviceName);
    }

    free(physicalDevices);
    ffvkDestroyInstance(vkInstance, NULL);
    dlclose(vulkan);
    return;
}

#endif

#ifdef FF_HAVE_LIBPCI
#include <pci/pci.h>

static void pciFillGPUs(FFinstance* instance, FFlist* results)
{
    FF_LIBRARY_LOAD(pci, "libpci.so", instance->config.libPCI,)

    FF_LIBRARY_LOAD_SYMBOL(pci, pci_alloc,)
    FF_LIBRARY_LOAD_SYMBOL(pci, pci_init,)
    FF_LIBRARY_LOAD_SYMBOL(pci, pci_scan_bus,)
    FF_LIBRARY_LOAD_SYMBOL(pci, pci_fill_info,)
    FF_LIBRARY_LOAD_SYMBOL(pci, pci_lookup_name,)
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
            ffpci_lookup_name(pacc, result->vendor.chars, result->vendor.allocated -1, PCI_LOOKUP_VENDOR, dev->vendor_id, dev->device_id);
            ffStrbufRecalculateLength(&result->vendor);

            ffStrbufInitA(&result->name, 256);
            ffpci_lookup_name(pacc, result->name.chars, result->name.allocated - 1, PCI_LOOKUP_DEVICE, dev->vendor_id, dev->device_id);
            ffStrbufRecalculateLength(&result->name);
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
    ffStrbufInitA(&gpu, result->vendor.length + namePretty.length);

    if(*vendorPretty != '\0')
    {
        ffStrbufAppendS(&gpu, vendorPretty);
        ffStrbufAppendC(&gpu, ' ');
    }

    ffStrbufAppend(&gpu, &namePretty);

    ffPrintAndAppendToCache(instance, FF_GPU_MODULE_NAME, index, &instance->config.gpuKey, cache, &gpu, &instance->config.gpuFormat, FF_GPU_NUM_FORMAT_ARGS, (FFformatarg[]){
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->vendor},
        {FF_FORMAT_ARG_TYPE_STRING, vendorPretty},
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->name},
        {FF_FORMAT_ARG_TYPE_STRBUF, &namePretty}
    });

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

    for(uint32_t i = 0; i < gpus.length; i++)
        printGPUResult(instance, gpus.length == 1 ? 0 : i + 1, &cache, ffListGet(&gpus, i));

    if(gpus.length == 0)
        ffPrintError(instance, FF_GPU_MODULE_NAME, 0, &instance->config.gpuKey, &instance->config.gpuFormat, FF_GPU_NUM_FORMAT_ARGS, "No GPUs found.");

    ffCacheClose(&cache);
    ffListDestroy(&gpus);
}

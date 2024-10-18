#include "fastfetch.h"
#include "detection/gpu/gpu.h"
#include "detection/vulkan/vulkan.h"

#ifdef FF_HAVE_VULKAN
#include "common/library.h"
#include "common/io/io.h"
#include "common/parsing.h"
#include "util/stringUtils.h"

#include <stdlib.h>
#include <vulkan/vulkan.h>

static inline void applyVulkanVersion(uint32_t vulkanVersion, FFVersion* ffVersion)
{
    ffVersion->major = VK_VERSION_MAJOR(vulkanVersion);
    ffVersion->minor = VK_VERSION_MINOR(vulkanVersion);
    ffVersion->patch = VK_VERSION_PATCH(vulkanVersion);
}

static void applyDriverName(VkPhysicalDeviceDriverPropertiesKHR* properties, FFstrbuf* result)
{
    if(!ffStrSet(properties->driverName))
        return;

    ffStrbufAppendS(result, properties->driverName);

    /*
     * Some drivers (android for example) expose a multiline string as driver info.
     * It contains too much info anyways, so we just don't append it.
     */
    if(!ffStrSet(properties->driverInfo) || strchr(properties->driverInfo, '\n') != NULL)
        return;

    ffStrbufAppendS(result, " [");
    ffStrbufAppendS(result, properties->driverInfo);
    ffStrbufAppendC(result, ']');
}

static const char* detectVulkan(FFVulkanResult* result)
{
    FF_LIBRARY_LOAD(vulkan, "dlopen libvulkan"FF_LIBRARY_EXTENSION " failed",
        #ifdef __APPLE__
            "libMoltenVK"FF_LIBRARY_EXTENSION, -1
        #elif defined(_WIN32)
            "vulkan-1"FF_LIBRARY_EXTENSION, -1
        #else
            "libvulkan"FF_LIBRARY_EXTENSION, 2
        #endif
    )
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE2(vulkan, vkGetInstanceProcAddr, vkGetInstanceProcAddr@8)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE2(vulkan, vkCreateInstance, vkCreateInstance@12)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE2(vulkan, vkDestroyInstance, vkDestroyInstance@8)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE2(vulkan, vkEnumeratePhysicalDevices, vkEnumeratePhysicalDevices@12)

    //Some drivers (nvdc) print messages to stdout
    //and that is the best way I found to disable that
    FF_SUPPRESS_IO();

    FFVersion instanceVersion = FF_VERSION_INIT;

    //We need to get the function pointer this way, because it is only provided by vulkan 1.1 and higher.
    //a dlsym would fail on 1.0 implementations
    PFN_vkEnumerateInstanceVersion ffvkEnumerateInstanceVersion = (PFN_vkEnumerateInstanceVersion) ffvkGetInstanceProcAddr(NULL, "vkEnumerateInstanceVersion");
    if(ffvkEnumerateInstanceVersion != NULL)
    {
        uint32_t version;
        if(ffvkEnumerateInstanceVersion(&version) == VK_SUCCESS)
            applyVulkanVersion(version, &instanceVersion);
    }

    const uint32_t projectVersion = VK_MAKE_VERSION(
        FASTFETCH_PROJECT_VERSION_MAJOR,
        FASTFETCH_PROJECT_VERSION_MINOR,
        FASTFETCH_PROJECT_VERSION_PATCH
    );

    VkInstance vkInstance;
    VkResult res = ffvkCreateInstance(&(VkInstanceCreateInfo) {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = NULL,
        .pApplicationInfo = &(VkApplicationInfo) {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = NULL,
            .pApplicationName = FASTFETCH_PROJECT_NAME,
            .applicationVersion = projectVersion,
            .pEngineName = "vulkanPrintGPUs",
            .engineVersion = projectVersion,

            // We need to request 1.1 to get physicalDeviceDriverProperties
            .apiVersion = instanceVersion.minor >= 1 ? VK_API_VERSION_1_1 : VK_API_VERSION_1_0
        },
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = NULL,
        .enabledExtensionCount = 0,
        .ppEnabledExtensionNames = NULL,
        .flags = 0
    }, NULL, &vkInstance);
    if(res != VK_SUCCESS)
    {
        switch (res)
        {
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                return "ffvkCreateInstance() failed: VK_ERROR_OUT_OF_HOST_MEMORY";
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                return "ffvkCreateInstance() failed: VK_ERROR_OUT_OF_DEVICE_MEMORY";
            case VK_ERROR_INITIALIZATION_FAILED:
                return "ffvkCreateInstance() failed: VK_ERROR_INITIALIZATION_FAILED";
            case VK_ERROR_LAYER_NOT_PRESENT:
                return "ffvkCreateInstance() failed: VK_ERROR_LAYER_NOT_PRESENT";
            case VK_ERROR_EXTENSION_NOT_PRESENT:
                return "ffvkCreateInstance() failed: VK_ERROR_EXTENSION_NOT_PRESENT";
            case VK_ERROR_INCOMPATIBLE_DRIVER:
                return "ffvkCreateInstance() failed: VK_ERROR_INCOMPATIBLE_DRIVER";
            default:
                return "ffvkCreateInstance() failed: unknown error";
        }
    }

    //if instance creation succeeded, but vkEnumerateInstanceVersion didn't, this means we are running against a vulkan 1.0 implementation
    //explicitly set this version, if no device is found, so we still have at least this info
    if(instanceVersion.major == 0 && instanceVersion.minor == 0 && instanceVersion.patch == 0)
        instanceVersion.major = 1;

    VkPhysicalDevice physicalDevices[128];
    uint32_t physicalDeviceCount = (uint32_t) ARRAY_SIZE(physicalDevices);
    res = ffvkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, physicalDevices);
    if(res != VK_SUCCESS)
    {
        ffvkDestroyInstance(vkInstance, NULL);
        switch (res)
        {
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            return "ffvkEnumeratePhysicalDevices() failed: VK_ERROR_OUT_OF_HOST_MEMORY";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            return "ffvkEnumeratePhysicalDevices() failed: VK_ERROR_OUT_OF_DEVICE_MEMORY";
        case VK_ERROR_INITIALIZATION_FAILED:
            return "ffvkEnumeratePhysicalDevices() failed: VK_ERROR_INITIALIZATION_FAILED";
        case VK_INCOMPLETE:
            return "ffvkEnumeratePhysicalDevices() failed: VK_INCOMPLETE";
        default:
            return "ffvkEnumeratePhysicalDevices() failed";
        }
    }

    PFN_vkGetPhysicalDeviceProperties ffvkGetPhysicalDeviceProperties = NULL;
    PFN_vkGetPhysicalDeviceProperties2 ffvkGetPhysicalDeviceProperties2 = (PFN_vkGetPhysicalDeviceProperties2) ffvkGetInstanceProcAddr(vkInstance, "vkGetPhysicalDeviceProperties2"); // 1.1
    if(!ffvkGetPhysicalDeviceProperties2)
        ffvkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties) ffvkGetInstanceProcAddr(vkInstance, "vkGetPhysicalDeviceProperties");

    PFN_vkGetPhysicalDeviceMemoryProperties ffvkGetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties) ffvkGetInstanceProcAddr(vkInstance, "vkGetPhysicalDeviceMemoryProperties");

    FFVersion maxDeviceApiVersion = FF_VERSION_INIT;
    FFVersion maxDeviceConformanceVersion = FF_VERSION_INIT;

    for(uint32_t i = 0; i < physicalDeviceCount; i++)
    {
        //Get device properties.
        //On VK 1.1 and up, we use vkGetPhysicalDeviceProperties2, so we can put VkPhysicalDeviceDriverProperties in the pNext chain.
        //This is required to get the driver name and conformance version.

        VkPhysicalDeviceDriverPropertiesKHR driverProperties = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES_KHR,
        };
        VkPhysicalDeviceProperties2 physicalDeviceProperties = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
            .pNext = &driverProperties,
        };

        if(ffvkGetPhysicalDeviceProperties2 != NULL)
            ffvkGetPhysicalDeviceProperties2(physicalDevices[i], &physicalDeviceProperties);
        else
            ffvkGetPhysicalDeviceProperties(physicalDevices[i], &physicalDeviceProperties.properties);


        //We don't want software rasterizers to show up as physical gpu
        if(physicalDeviceProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU)
            continue;

        //If the device api version is higher than the current highest device api version, overwrite it
        //In this case, also use the current device driver name as the shown driver name

        FFVersion deviceAPIVersion = FF_VERSION_INIT;
        applyVulkanVersion(physicalDeviceProperties.properties.apiVersion, &deviceAPIVersion);
        if(ffVersionCompare(&deviceAPIVersion, &maxDeviceApiVersion) > 0)
        {
            maxDeviceApiVersion = deviceAPIVersion;
            applyDriverName(&driverProperties, &result->driver);
        }

        //If the device conformance version is higher than the current highest device conformance version, overwrite it
        if(ffvkGetPhysicalDeviceProperties2)
        {
            FFVersion deviceConformanceVersion = {
                .major = driverProperties.conformanceVersion.major,
                .minor = driverProperties.conformanceVersion.minor,
                .patch = driverProperties.conformanceVersion.patch,
            };

            if(ffVersionCompare(&deviceConformanceVersion, &maxDeviceConformanceVersion) > 0)
                maxDeviceConformanceVersion = deviceConformanceVersion;
        }

        //Add the device to the list of devices shown by the GPU module

        // #456
        FF_LIST_FOR_EACH(FFGPUResult, gpu, result->gpus)
        {
            if (gpu->deviceId == physicalDeviceProperties.properties.deviceID)
                goto next;
        }

        FFGPUResult* gpu = ffListAdd(&result->gpus);

        ffStrbufInitF(&gpu->platformApi, "Vulkan %u.%u.%u", deviceAPIVersion.major, deviceAPIVersion.minor, deviceAPIVersion.patch);
        gpu->deviceId = physicalDeviceProperties.properties.deviceID;

        ffStrbufInitS(&gpu->name, physicalDeviceProperties.properties.deviceName);

        gpu->type = physicalDeviceProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? FF_GPU_TYPE_DISCRETE : FF_GPU_TYPE_INTEGRATED;
        ffStrbufInitS(&gpu->vendor, ffGetGPUVendorString(physicalDeviceProperties.properties.vendorID));
        ffStrbufInitS(&gpu->driver, driverProperties.driverInfo);

        VkPhysicalDeviceMemoryProperties memoryProperties = {};
        ffvkGetPhysicalDeviceMemoryProperties(physicalDevices[i], &memoryProperties);

        gpu->dedicated.total = gpu->shared.total = 0;
        gpu->dedicated.used = gpu->shared.used = FF_GPU_VMEM_SIZE_UNSET;
        for(uint32_t index = 0; index < memoryProperties.memoryHeapCount; ++index)
        {
            const VkMemoryHeap* heap = &memoryProperties.memoryHeaps[index];
            FFGPUMemory* vmem = gpu->type == FF_GPU_TYPE_DISCRETE && (heap->flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) ? &gpu->dedicated : &gpu->shared;
            vmem->total += heap->size;
        }

        //No way to detect those using vulkan
        gpu->index = FF_GPU_INDEX_UNSET;
        gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
        gpu->temperature = FF_GPU_TEMP_UNSET;
        gpu->frequency = FF_GPU_FREQUENCY_UNSET;
        gpu->coreUsage = FF_GPU_CORE_USAGE_UNSET;

    next:
        continue;
    }

    ffVersionToPretty(&instanceVersion, &result->instanceVersion);
    ffVersionToPretty(&maxDeviceApiVersion, &result->apiVersion);
    ffVersionToPretty(&maxDeviceConformanceVersion, &result->conformanceVersion);

    ffvkDestroyInstance(vkInstance, NULL);
    return NULL;
}

#endif

FFVulkanResult* ffDetectVulkan(void)
{
    static FFVulkanResult result;

    if (result.gpus.elementSize == 0)
    {
        ffStrbufInit(&result.driver);
        ffStrbufInit(&result.apiVersion);
        ffStrbufInit(&result.conformanceVersion);
        ffStrbufInit(&result.instanceVersion);
        ffListInit(&result.gpus, sizeof(FFGPUResult));

        #ifdef FF_HAVE_VULKAN
            result.error = detectVulkan(&result);
        #else
            result.error = "fastfetch was compiled without vulkan support";
        #endif
    }

    return &result;
}

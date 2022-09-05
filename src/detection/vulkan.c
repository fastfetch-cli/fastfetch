#include "fastfetch.h"
#include "detection/vulkan.h"
#include "detection/gpu/gpu.h"

#include <pthread.h>

#ifdef FF_HAVE_VULKAN
#include "common/library.h"
#include "common/io.h"
#include "common/parsing.h"
#include <stdlib.h>
#include <vulkan/vulkan.h>

static inline void applyVulkanVersion(uint32_t vulkanVersion, FFVersion* ffVersion)
{
    ffVersion->major = VK_VERSION_MAJOR(vulkanVersion);
    ffVersion->minor = VK_VERSION_MINOR(vulkanVersion);
    ffVersion->patch = VK_VERSION_PATCH(vulkanVersion);
}

static void applyDriverName(VkPhysicalDeviceDriverProperties* properties, FFstrbuf* result)
{
    if(!ffStrSet(properties->driverName))
        return;

    ffStrbufAppendS(result, properties->driverName);

    if(!ffStrSet(properties->driverInfo))
        return;

    ffStrbufAppendS(result, " [");
    ffStrbufAppendS(result, properties->driverInfo);
    ffStrbufAppendC(result, ']');
}

static void detectVulkan(const FFinstance* instance, FFVulkanResult* result)
{
    FF_LIBRARY_LOAD(vulkan, instance->config.libVulkan, , "libvulkan.so", 2)
    FF_LIBRARY_LOAD_SYMBOL(vulkan, vkGetInstanceProcAddr,)
    FF_LIBRARY_LOAD_SYMBOL(vulkan, vkCreateInstance,)
    FF_LIBRARY_LOAD_SYMBOL(vulkan, vkDestroyInstance,)
    FF_LIBRARY_LOAD_SYMBOL(vulkan, vkEnumeratePhysicalDevices,)
    FF_LIBRARY_LOAD_SYMBOL(vulkan, vkGetPhysicalDeviceProperties,)

    //Some drivers (nvdc) print messages to stdout
    //and thats the best way i found to disable that
    ffSuppressIO(true);

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

    //We need to request 1.2 to get physicalDeviceDriverProperties
    uint32_t requestedVkVersion = VK_API_VERSION_1_0;
    if(instanceVersion.minor >= 2)
        requestedVkVersion = VK_API_VERSION_1_2;

    const VkApplicationInfo applicationInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = NULL,
        .pApplicationName = FASTFETCH_PROJECT_NAME,
        .applicationVersion = projectVersion,
        .pEngineName = "vulkanPrintGPUs",
        .engineVersion = projectVersion,
        .apiVersion = requestedVkVersion
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


    //if instance creation succeeded, but vkEnumerateInstanceVersion didn't, this means we are running against a vulkan 1.0 implementation
    //explicitly set this version, if no device is found, so we still have at least this info
    if(instanceVersion.major == 0 && instanceVersion.minor == 0 && instanceVersion.patch == 0)
        instanceVersion.major = 1;

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

    PFN_vkGetPhysicalDeviceProperties2 ffvkGetPhysicalDeviceProperties2 = (PFN_vkGetPhysicalDeviceProperties2) ffvkGetInstanceProcAddr(vkInstance, "vkGetPhysicalDeviceProperties2");

    FFVersion maxDeviceApiVersion = FF_VERSION_INIT;
    FFVersion maxDeviceConformanceVersion = FF_VERSION_INIT;

    for(uint32_t i = 0; i < physicalDeviceCount; i++)
    {
        //Get device properties.
        //On VK 1.2 and up, we use vkGetPhysicalDeviceProperties2, so we can put VkPhysicalDeviceDriverProperties in the pNext chain.
        //This is required to get the driver name and conformance version.

        VkPhysicalDeviceDriverProperties driverProperties;
        driverProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES;
        driverProperties.pNext = NULL;
        driverProperties.driverName[0] = '\0';
        driverProperties.conformanceVersion = (VkConformanceVersion) {0, 0, 0, 0};

        VkPhysicalDeviceProperties2 physicalDeviceProperties;
        physicalDeviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        physicalDeviceProperties.pNext = &driverProperties;

        if(ffvkGetPhysicalDeviceProperties2 != NULL)
            ffvkGetPhysicalDeviceProperties2(physicalDevices[i], &physicalDeviceProperties);
        else
            ffvkGetPhysicalDeviceProperties(physicalDevices[i], &physicalDeviceProperties.properties);

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

        FFVersion deviceConformanceVersion = FF_VERSION_INIT;
        deviceConformanceVersion.major = driverProperties.conformanceVersion.major;
        deviceConformanceVersion.minor = driverProperties.conformanceVersion.minor;
        deviceConformanceVersion.patch = driverProperties.conformanceVersion.patch;

        if(ffVersionCompare(&deviceConformanceVersion, &maxDeviceConformanceVersion) > 0)
            maxDeviceConformanceVersion = deviceConformanceVersion;

        //Add the device to the list of devices shown by the GPU module

        //We don't want softare rasterizers to show up as physical gpu
        if(physicalDeviceProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU)
            continue;

        FFGPUResult* gpu = ffListAdd(&result->gpus);

        ffStrbufInit(&gpu->name);
        ffStrbufAppendS(&gpu->name, physicalDeviceProperties.properties.deviceName);

        //No way to detect those using vulkan
        ffStrbufInit(&gpu->vendor);
        ffStrbufInit(&gpu->driver);
        gpu->temperature = FF_GPU_TEMP_UNSET;
    }

    //If the highest device version is lower than the instance version, use it as our vulkan version
    //otherwise the instance version is the vulkan version
    if(ffVersionCompare(&instanceVersion, &maxDeviceApiVersion) > 0)
        ffVersionToPretty(&maxDeviceApiVersion, &result->apiVersion);
    else
        ffVersionToPretty(&instanceVersion, &result->apiVersion);

    //Use the highest device conformace version as our conformance version
    ffVersionToPretty(&maxDeviceConformanceVersion, &result->conformanceVersion);

    free(physicalDevices);
    ffvkDestroyInstance(vkInstance, NULL);
    dlclose(vulkan);
    ffSuppressIO(false);
}

#endif

const FFVulkanResult* ffDetectVulkan(const FFinstance* instance)
{
    static FFVulkanResult result;
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    static bool init = false;

    pthread_mutex_lock(&mutex);
    if(init)
    {
        pthread_mutex_unlock(&mutex);
        return &result;
    }
    init = true;

    ffStrbufInit(&result.driver);
    ffStrbufInit(&result.apiVersion);
    ffStrbufInit(&result.conformanceVersion);
    ffListInit(&result.gpus, sizeof(FFGPUResult));

    #ifdef FF_HAVE_VULKAN
        detectVulkan(instance, &result);
    #else
        FF_UNUSED(instance);
    #endif

    pthread_mutex_unlock(&mutex);
    return &result;
}

#include "codec.h"

#ifdef FF_HAVE_VULKAN
    #include <vulkan/vulkan.h>
    #include "common/strutil.h"

    #if VK_KHR_video_decode_queue && VK_KHR_video_encode_queue

        #include "common/debug.h"
        #include "common/io.h"
        #include "common/library.h"
        #include "common/mallocHelper.h"

static FFCodecType ffCodecDecodeOperationsToTypes(VkVideoCodecOperationFlagsKHR operations) {
    FFCodecType types = FF_CODEC_TYPE_NONE;

    if (operations & VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR) {
        types |= FF_CODEC_TYPE_H264;
    }
    if (operations & VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR) {
        types |= FF_CODEC_TYPE_HEVC;
    }
    if (operations & VK_VIDEO_CODEC_OPERATION_DECODE_AV1_BIT_KHR) {
        types |= FF_CODEC_TYPE_AV1;
    }
    if (operations & 0x00000008 /*VK_VIDEO_CODEC_OPERATION_DECODE_VP9_BIT_KHR*/) {
        types |= FF_CODEC_TYPE_VP9;
    }

    return types;
}

static FFCodecType ffCodecEncodeOperationsToTypes(VkVideoCodecOperationFlagsKHR operations) {
    FFCodecType types = FF_CODEC_TYPE_NONE;

    if (operations & VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR) {
        types |= FF_CODEC_TYPE_H264;
    }
    if (operations & VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR) {
        types |= FF_CODEC_TYPE_HEVC;
    }
    if (operations & VK_VIDEO_CODEC_OPERATION_ENCODE_AV1_BIT_KHR) {
        types |= FF_CODEC_TYPE_AV1;
    }

    return types;
}

static bool ffCodecHasDeviceExtension(const VkExtensionProperties* extensions, uint32_t extensionCount, const char* extensionName) {
    for (uint32_t i = 0; i < extensionCount; ++i) {
        if (ffStrEquals(extensions[i].extensionName, extensionName)) {
            return true;
        }
    }

    return false;
}

const char* ffDetectCodecVulkan(FFCodecOptions* options, FFlist* result /*list of FFCodecResult*/) {
    FF_DEBUG("Starting Vulkan codec detection");

    FF_LIBRARY_LOAD_MESSAGE(vulkan,
        #if __APPLE__
        "libMoltenVK" FF_LIBRARY_EXTENSION,
        -1
        #elif _WIN32
        "vulkan-1" FF_LIBRARY_EXTENSION,
        -1
        #else
        "libvulkan" FF_LIBRARY_EXTENSION,
        2
        #endif
    )
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(vulkan, vkGetInstanceProcAddr)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(vulkan, vkCreateInstance)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(vulkan, vkDestroyInstance)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(vulkan, vkEnumeratePhysicalDevices)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(vulkan, vkEnumerateDeviceExtensionProperties)

    FF_SUPPRESS_IO();

    uint32_t apiVersion = VK_API_VERSION_1_0;
    PFN_vkEnumerateInstanceVersion ffvkEnumerateInstanceVersion = (PFN_vkEnumerateInstanceVersion) ffvkGetInstanceProcAddr(NULL, "vkEnumerateInstanceVersion");
    if (ffvkEnumerateInstanceVersion != NULL) {
        uint32_t detectedApiVersion = 0;
        if (ffvkEnumerateInstanceVersion(&detectedApiVersion) == VK_SUCCESS) {
            apiVersion = detectedApiVersion;
        }
    }

    const uint32_t projectVersion = VK_MAKE_VERSION(
        FASTFETCH_PROJECT_VERSION_MAJOR,
        FASTFETCH_PROJECT_VERSION_MINOR,
        FASTFETCH_PROJECT_VERSION_PATCH);

    VkInstance vkInstance = VK_NULL_HANDLE;
    VkResult res = ffvkCreateInstance(&(VkInstanceCreateInfo){
                                          .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                          .pApplicationInfo = &(VkApplicationInfo){
                                              .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                                              .pApplicationName = FASTFETCH_PROJECT_NAME,
                                              .applicationVersion = projectVersion,
                                              .pEngineName = "fastfetch-codec-vulkan",
                                              .engineVersion = projectVersion,
                                              .apiVersion = apiVersion,
                                          },
                                      },
        NULL,
        &vkInstance);
    if (res != VK_SUCCESS) {
        FF_DEBUG("ffvkCreateInstance() failed with VkResult=%d", res);
        switch (res) {
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

    PFN_vkGetPhysicalDeviceProperties ffvkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties) ffvkGetInstanceProcAddr(vkInstance, "vkGetPhysicalDeviceProperties");
    if (!ffvkGetPhysicalDeviceProperties) {
        ffvkDestroyInstance(vkInstance, NULL);
        return "vkGetPhysicalDeviceProperties is not available";
    }

    PFN_vkGetPhysicalDeviceQueueFamilyProperties2 ffvkGetPhysicalDeviceQueueFamilyProperties2 = (PFN_vkGetPhysicalDeviceQueueFamilyProperties2) ffvkGetInstanceProcAddr(vkInstance, "vkGetPhysicalDeviceQueueFamilyProperties2");
    if (!ffvkGetPhysicalDeviceQueueFamilyProperties2) {
        ffvkDestroyInstance(vkInstance, NULL);
        return "vkGetPhysicalDeviceQueueFamilyProperties2 is not available";
    }

    uint32_t physicalDeviceCount = 0;
    res = ffvkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, NULL);
    if (res != VK_SUCCESS) {
        FF_DEBUG("ffvkEnumeratePhysicalDevices(count) failed with VkResult=%d", res);
        ffvkDestroyInstance(vkInstance, NULL);
        return "ffvkEnumeratePhysicalDevices() failed during Vulkan codec detection";
    }
    if (physicalDeviceCount == 0) {
        ffvkDestroyInstance(vkInstance, NULL);
        return "No Vulkan physical devices found";
    }

    FF_AUTO_FREE VkPhysicalDevice* physicalDevices = (VkPhysicalDevice*) malloc(sizeof(VkPhysicalDevice) * (size_t) physicalDeviceCount);

    res = ffvkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, physicalDevices);
    if (res != VK_SUCCESS) {
        FF_DEBUG("ffvkEnumeratePhysicalDevices(list) failed with VkResult=%d", res);
        ffvkDestroyInstance(vkInstance, NULL);
        return "ffvkEnumeratePhysicalDevices() failed during Vulkan codec detection";
    }

    bool sawVideoQueueExtension = false;

    for (uint32_t i = 0; i < physicalDeviceCount; ++i) {
        VkPhysicalDeviceProperties properties = {};
        ffvkGetPhysicalDeviceProperties(physicalDevices[i], &properties);

        uint32_t extensionCount = 0;
        res = ffvkEnumerateDeviceExtensionProperties(physicalDevices[i], NULL, &extensionCount, NULL);
        if (res != VK_SUCCESS) {
            FF_DEBUG("vkEnumerateDeviceExtensionProperties(count) failed for '%s' with VkResult=%d", properties.deviceName, res);
            continue;
        }

        if (extensionCount == 0) {
            FF_DEBUG("Skipping Vulkan device '%s' because it has no extensions", properties.deviceName);
            continue;
        }

        FF_AUTO_FREE VkExtensionProperties* extensions = (VkExtensionProperties*) malloc(sizeof(VkExtensionProperties) * (size_t) extensionCount);

        res = ffvkEnumerateDeviceExtensionProperties(physicalDevices[i], NULL, &extensionCount, extensions);
        if (res != VK_SUCCESS) {
            FF_DEBUG("vkEnumerateDeviceExtensionProperties(list) failed for '%s' with VkResult=%d", properties.deviceName, res);
            continue;
        }

        bool hasVideoDecode = (options->showType & FF_CODEC_SHOW_TYPE_DECODER) &&
            ffCodecHasDeviceExtension(extensions, extensionCount, VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME);
        bool hasVideoEncode = (options->showType & FF_CODEC_SHOW_TYPE_ENCODER) &&
            ffCodecHasDeviceExtension(extensions, extensionCount, VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME);

        if (!hasVideoDecode && !hasVideoEncode) {
            FF_DEBUG("Skipping Vulkan device '%s' because it does not support video queue extensions", properties.deviceName);
            continue;
        }
        sawVideoQueueExtension = true;

        uint32_t queueFamilyCount = 0;
        ffvkGetPhysicalDeviceQueueFamilyProperties2(physicalDevices[i], &queueFamilyCount, NULL);
        if (queueFamilyCount == 0) {
            FF_DEBUG("Skipping Vulkan device '%s' because it has no queue families", properties.deviceName);
            continue;
        }

        FF_AUTO_FREE VkQueueFamilyProperties2* queueFamilyProperties = (VkQueueFamilyProperties2*) malloc(sizeof(VkQueueFamilyProperties2) * (size_t) queueFamilyCount);
        FF_AUTO_FREE VkQueueFamilyVideoPropertiesKHR* queueFamilyVideoProperties = (VkQueueFamilyVideoPropertiesKHR*) malloc(sizeof(VkQueueFamilyVideoPropertiesKHR) * (size_t) queueFamilyCount);

        for (uint32_t queueIndex = 0; queueIndex < queueFamilyCount; ++queueIndex) {
            queueFamilyVideoProperties[queueIndex] = (VkQueueFamilyVideoPropertiesKHR){
                .sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_VIDEO_PROPERTIES_KHR,
            };
            queueFamilyProperties[queueIndex] = (VkQueueFamilyProperties2){
                .sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2,
                .pNext = &queueFamilyVideoProperties[queueIndex],
            };
        }

        ffvkGetPhysicalDeviceQueueFamilyProperties2(physicalDevices[i], &queueFamilyCount, queueFamilyProperties);

        FFCodecType decoders = FF_CODEC_TYPE_NONE;
        FFCodecType encoders = FF_CODEC_TYPE_NONE;

        for (uint32_t queueIndex = 0; queueIndex < queueFamilyCount; ++queueIndex) {
            const VkQueueFlags queueFlags = queueFamilyProperties[queueIndex].queueFamilyProperties.queueFlags;
            const VkVideoCodecOperationFlagsKHR operations = queueFamilyVideoProperties[queueIndex].videoCodecOperations;

            if (hasVideoDecode && (queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR)) {
                decoders |= ffCodecDecodeOperationsToTypes(operations);
            }

            if (hasVideoEncode && (queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR)) {
                encoders |= ffCodecEncodeOperationsToTypes(operations);
            }
        }

        if (decoders == FF_CODEC_TYPE_NONE && encoders == FF_CODEC_TYPE_NONE) {
            FF_DEBUG("Skipping Vulkan device '%s' because no supported codec operations were reported", properties.deviceName);
            continue;
        }

        FFCodecResult* item = FF_LIST_ADD(FFCodecResult, *result);
        ffStrbufInitS(&item->gpu, properties.deviceName);
        item->decoders = decoders;
        item->encoders = encoders;
        item->platformApi = "Vulkan Video";
        FF_DEBUG("Added Vulkan codec result for '%s': decoders=%u encoders=%u", properties.deviceName, (unsigned) decoders, (unsigned) encoders);
    }

    ffvkDestroyInstance(vkInstance, NULL);

    if (result->length > 0) {
        return NULL;
    }

    return sawVideoQueueExtension ? "No supported Vulkan video codec operations found"
                                  : "VK_KHR_video_queue is not supported by any Vulkan physical device";
}

    #else

const char* ffDetectCodecVulkan(FFCodecOptions* options, FFlist* result) {
    FF_UNUSED(options, result);
    return "Vulkan video queue extensions are not supported by this Vulkan implementation";
}

    #endif

#endif

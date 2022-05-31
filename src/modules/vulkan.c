#include "fastfetch.h"

#define FF_VULKAN_MODULE_NAME "Vulkan"
#define FF_VULKAN_NUM_FORMAT_ARGS 3

void ffPrintVulkan(FFinstance* instance)
{
    const FFVulkanResult* vulkan = ffDetectVulkan(instance);

    if(vulkan->apiVersion.length == 0 && vulkan->driver.length == 0)
    {
        ffPrintError(instance, FF_VULKAN_MODULE_NAME, 0, &instance->config.vulkanKey, &instance->config.vulkanFormat, FF_VULKAN_NUM_FORMAT_ARGS, "Failed to detect vulkan");
        return;
    }

    if(instance->config.vulkanFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_VULKAN_MODULE_NAME, 0, &instance->config.vulkanKey);

        if(vulkan->driver.length > 0)
        {
            printf("%s (driver)", vulkan->driver.chars);

            if(vulkan->apiVersion.length > 0)
                fputs(", ", stdout);
        }

        if(vulkan->apiVersion.length > 0)
            printf("%s (api version)", vulkan->apiVersion.chars);

        putchar('\n');
    }
    else
    {
        ffPrintFormatString(instance, FF_VULKAN_MODULE_NAME, 0, &instance->config.vulkanKey, &instance->config.vulkanFormat, NULL, FF_VULKAN_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRBUF, &vulkan->driver},
            {FF_FORMAT_ARG_TYPE_STRBUF, &vulkan->apiVersion},
            {FF_FORMAT_ARG_TYPE_STRBUF, &vulkan->conformanceVersion}
        });
    }
}

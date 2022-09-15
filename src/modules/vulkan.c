#include "fastfetch.h"
#include "common/printing.h"
#include "detection/vulkan.h"

#define FF_VULKAN_MODULE_NAME "Vulkan"
#define FF_VULKAN_NUM_FORMAT_ARGS 3

void ffPrintVulkan(FFinstance* instance)
{
    const FFVulkanResult* vulkan = ffDetectVulkan(instance);

    if(vulkan->error)
    {
        ffPrintError(instance, FF_VULKAN_MODULE_NAME, 0, &instance->config.vulkan, vulkan->error);
        return;
    }

    if(instance->config.vulkan.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_VULKAN_MODULE_NAME, 0, &instance->config.vulkan.key);

        if(vulkan->apiVersion.length > 0)
        {
            ffStrbufWriteTo(&vulkan->apiVersion, stdout);

            if(vulkan->driver.length > 0)
                fputs(" - ", stdout);
        }

        if(vulkan->driver.length > 0)
            ffStrbufWriteTo(&vulkan->driver, stdout);

        putchar('\n');
    }
    else
    {
        ffPrintFormat(instance, FF_VULKAN_MODULE_NAME, 0, &instance->config.vulkan, FF_VULKAN_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRBUF, &vulkan->driver},
            {FF_FORMAT_ARG_TYPE_STRBUF, &vulkan->apiVersion},
            {FF_FORMAT_ARG_TYPE_STRBUF, &vulkan->conformanceVersion}
        });
    }
}

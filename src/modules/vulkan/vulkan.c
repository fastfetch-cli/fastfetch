#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/vulkan/vulkan.h"
#include "modules/vulkan/vulkan.h"
#include "util/stringUtils.h"

#define FF_VULKAN_NUM_FORMAT_ARGS 3

void ffPrintVulkan(FFVulkanOptions* options)
{
    const FFVulkanResult* vulkan = ffDetectVulkan();

    if(vulkan->error)
    {
        ffPrintError(FF_VULKAN_MODULE_NAME, 0, &options->moduleArgs, "%s", vulkan->error);
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_VULKAN_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

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
        ffPrintFormat(FF_VULKAN_MODULE_NAME, 0, &options->moduleArgs, FF_VULKAN_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRBUF, &vulkan->driver},
            {FF_FORMAT_ARG_TYPE_STRBUF, &vulkan->apiVersion},
            {FF_FORMAT_ARG_TYPE_STRBUF, &vulkan->conformanceVersion}
        });
    }
}

void ffInitVulkanOptions(FFVulkanOptions* options)
{
    ffOptionInitModuleBaseInfo(&options->moduleInfo, FF_VULKAN_MODULE_NAME, ffParseVulkanCommandOptions, ffParseVulkanJsonObject, ffPrintVulkan, NULL);
    ffOptionInitModuleArg(&options->moduleArgs);
}

bool ffParseVulkanCommandOptions(FFVulkanOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_VULKAN_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroyVulkanOptions(FFVulkanOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

void ffParseVulkanJsonObject(FFVulkanOptions* options, yyjson_val* module)
{
    yyjson_val *key_, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key_, val)
    {
        const char* key = yyjson_get_str(key_);
        if(ffStrEqualsIgnCase(key, "type"))
            continue;

        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_VULKAN_MODULE_NAME, 0, &options->moduleArgs, "Unknown JSON key %s", key);
    }
}

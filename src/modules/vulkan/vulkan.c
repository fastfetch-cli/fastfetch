#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/vulkan/vulkan.h"
#include "modules/vulkan/vulkan.h"

#define FF_VULKAN_NUM_FORMAT_ARGS 3

void ffPrintVulkan(FFinstance* instance, FFVulkanOptions* options)
{
    const FFVulkanResult* vulkan = ffDetectVulkan(instance);

    if(vulkan->error)
    {
        ffPrintError(instance, FF_VULKAN_MODULE_NAME, 0, &options->moduleArgs, "%s", vulkan->error);
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_VULKAN_MODULE_NAME, 0, &options->moduleArgs.key, &options->moduleArgs.keyColor);

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
        ffPrintFormat(instance, FF_VULKAN_MODULE_NAME, 0, &options->moduleArgs, FF_VULKAN_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRBUF, &vulkan->driver},
            {FF_FORMAT_ARG_TYPE_STRBUF, &vulkan->apiVersion},
            {FF_FORMAT_ARG_TYPE_STRBUF, &vulkan->conformanceVersion}
        });
    }
}

void ffInitVulkanOptions(FFVulkanOptions* options)
{
    options->moduleName = FF_VULKAN_MODULE_NAME;
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

void ffParseVulkanJsonObject(FFinstance* instance, yyjson_val* module)
{
    FFVulkanOptions __attribute__((__cleanup__(ffDestroyVulkanOptions))) options;
    ffInitVulkanOptions(&options);

    if (module)
    {
        yyjson_val *key_, *val;
        size_t idx, max;
        yyjson_obj_foreach(module, idx, max, key_, val)
        {
            const char* key = yyjson_get_str(key_);
            if(strcasecmp(key, "type") == 0)
                continue;

            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            ffPrintError(instance, FF_VULKAN_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintVulkan(instance, &options);
}

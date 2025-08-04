#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/gpu/gpu.h"
#include "detection/vulkan/vulkan.h"
#include "modules/vulkan/vulkan.h"
#include "util/stringUtils.h"

void ffPrintVulkan(FFVulkanOptions* options)
{
    const FFVulkanResult* vulkan = ffDetectVulkan();

    if(vulkan->error)
    {
        ffPrintError(FF_VULKAN_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", vulkan->error);
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_VULKAN_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

        if (vulkan->apiVersion.length == 0 && vulkan->driver.length == 0)
        {
            ffStrbufWriteTo(&vulkan->instanceVersion, stdout);
            puts(" [Software only]");
            return;
        }

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
        FF_PRINT_FORMAT_CHECKED(FF_VULKAN_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]) {
            FF_FORMAT_ARG(vulkan->driver, "driver"),
            FF_FORMAT_ARG(vulkan->apiVersion, "api-version"),
            FF_FORMAT_ARG(vulkan->conformanceVersion, "conformance-version"),
            FF_FORMAT_ARG(vulkan->instanceVersion, "instance-version"),
        }));
    }
}

void ffParseVulkanJsonObject(FFVulkanOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_VULKAN_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateVulkanJsonConfig(FFVulkanOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyVulkanOptions))) FFVulkanOptions defaultOptions;
    ffInitVulkanOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
}

void ffGenerateVulkanJsonResult(FF_MAYBE_UNUSED FFVulkanOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    const FFVulkanResult* result = ffDetectVulkan();

    if(result->error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", result->error);
        return;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, obj, "apiVersion", &result->apiVersion);
    yyjson_mut_obj_add_strbuf(doc, obj, "conformanceVersion", &result->conformanceVersion);
    yyjson_mut_obj_add_strbuf(doc, obj, "driver", &result->driver);
    yyjson_mut_val* gpus = yyjson_mut_obj_add_arr(doc, obj, "gpus");
    FF_LIST_FOR_EACH(FFGPUResult, vulkanGpu, result->gpus)
    {
        yyjson_mut_val* gpuObj = yyjson_mut_arr_add_obj(doc, gpus);
        yyjson_mut_obj_add_str(doc, gpuObj, "type", vulkanGpu->type == FF_GPU_TYPE_UNKNOWN ? "Unknown" : vulkanGpu->type == FF_GPU_TYPE_INTEGRATED ? "Integrated" : "Discrete");
        yyjson_mut_obj_add_strbuf(doc, gpuObj, "vendor", &vulkanGpu->vendor);
        yyjson_mut_obj_add_strbuf(doc, gpuObj, "name", &vulkanGpu->name);
        yyjson_mut_obj_add_strbuf(doc, gpuObj, "driver", &vulkanGpu->driver);
        yyjson_mut_obj_add_strbuf(doc, gpuObj, "platformApi", &vulkanGpu->platformApi);
        yyjson_mut_obj_add_uint(doc, gpuObj, "deviceId", vulkanGpu->deviceId);

        yyjson_mut_val* memoryObj = yyjson_mut_obj_add_obj(doc, gpuObj, "memory");

        {
            yyjson_mut_val* dedicatedMemory = yyjson_mut_obj_add_obj(doc, memoryObj, "dedicated");
            if (vulkanGpu->dedicated.total != FF_GPU_VMEM_SIZE_UNSET)
                yyjson_mut_obj_add_uint(doc, dedicatedMemory, "total", vulkanGpu->dedicated.total);
            else
                yyjson_mut_obj_add_null(doc, dedicatedMemory, "total");

            if (vulkanGpu->dedicated.used != FF_GPU_VMEM_SIZE_UNSET)
                yyjson_mut_obj_add_uint(doc, dedicatedMemory, "used", vulkanGpu->dedicated.total);
            else
                yyjson_mut_obj_add_null(doc, dedicatedMemory, "used");
        }

        {
            yyjson_mut_val* sharedMemory = yyjson_mut_obj_add_obj(doc, memoryObj, "shared");
            if (vulkanGpu->shared.total != FF_GPU_VMEM_SIZE_UNSET)
                yyjson_mut_obj_add_uint(doc, sharedMemory, "total", vulkanGpu->shared.total);
            else
                yyjson_mut_obj_add_null(doc, sharedMemory, "total");

            if (vulkanGpu->shared.used != FF_GPU_VMEM_SIZE_UNSET)
                yyjson_mut_obj_add_uint(doc, sharedMemory, "used", vulkanGpu->shared.used);
            else
                yyjson_mut_obj_add_null(doc, sharedMemory, "used");
        }

        yyjson_mut_obj_add_uint(doc, gpuObj, "deviceId", vulkanGpu->deviceId);
    }
}

void ffInitVulkanOptions(FFVulkanOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "");
}

void ffDestroyVulkanOptions(FFVulkanOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffVulkanModuleInfo = {
    .name = FF_VULKAN_MODULE_NAME,
    .description = "Print highest Vulkan version supported by the GPU",
    .initOptions = (void*) ffInitVulkanOptions,
    .destroyOptions = (void*) ffDestroyVulkanOptions,
    .parseJsonObject = (void*) ffParseVulkanJsonObject,
    .printModule = (void*) ffPrintVulkan,
    .generateJsonResult = (void*) ffGenerateVulkanJsonResult,
    .generateJsonConfig = (void*) ffGenerateVulkanJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Driver name", "driver"},
        {"API version", "api-version"},
        {"Conformance version", "conformance-version"},
        {"Instance version", "instance-version"},
    }))
};

#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/cpucache/cpucache.h"
#include "modules/cpucache/cpucache.h"
#include "util/stringUtils.h"

#define FF_CPUCACHE_NUM_FORMAT_ARGS 4

void ffPrintCPUCache(FFCPUCacheOptions* options)
{
    FFCPUCacheResult result = {
        .caches = {
            ffListCreate(sizeof(FFCPUCache)),
            ffListCreate(sizeof(FFCPUCache)),
            ffListCreate(sizeof(FFCPUCache)),
            ffListCreate(sizeof(FFCPUCache)),
        },
    };

    const char* error = ffDetectCPUCache(&result);

    if(error)
    {
        ffPrintError(FF_CPUCACHE_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        goto exit;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        for (uint32_t i = 0; i < sizeof (result.caches) / sizeof (result.caches[0]) && result.caches[i].length > 0; i++)
        {
            FFCPUCache* src = (FFCPUCache*) &result.caches[i];

            char keys[32];
            snprintf(keys, sizeof(keys), "FF_CPUCACHE_MODULE_NAME (L%u)", (unsigned) i);
            ffPrintLogoAndKey(keys, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        }
        putchar('\n');
    }
    else
    {
        // FF_PRINT_FORMAT_CHECKED(FF_CPUCACHE_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_CPUCACHE_NUM_FORMAT_ARGS, ((FFformatarg[]) {
        //     {FF_FORMAT_ARG_TYPE_STRBUF, &result.type, "type"},
        //     {FF_FORMAT_ARG_TYPE_STRBUF, &result.vendor, "vendor"},
        //     {FF_FORMAT_ARG_TYPE_STRBUF, &result.version, "version"},
        //     {FF_FORMAT_ARG_TYPE_STRBUF, &result.serial, "serial"},
        // }));
    }

exit:
    ffListDestroy(&result.caches[0]);
    ffListDestroy(&result.caches[1]);
    ffListDestroy(&result.caches[2]);
    ffListDestroy(&result.caches[3]);
}

bool ffParseCPUCacheCommandOptions(FFCPUCacheOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_CPUCACHE_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffParseCPUCacheJsonObject(FFCPUCacheOptions* options, yyjson_val* module)
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

        ffPrintError(FF_CPUCACHE_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateCPUCacheJsonConfig(FFCPUCacheOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyCPUCacheOptions))) FFCPUCacheOptions defaultOptions;
    ffInitCPUCacheOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
}

void ffGenerateCPUCacheJsonResult(FF_MAYBE_UNUSED FFCPUCacheOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FFCPUCacheResult result = {
        .caches = {
            ffListCreate(sizeof(FFCPUCache)),
            ffListCreate(sizeof(FFCPUCache)),
            ffListCreate(sizeof(FFCPUCache)),
            ffListCreate(sizeof(FFCPUCache)),
        },
    };

    const char* error = ffDetectCPUCache(&result);

    if (error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        goto exit;
    }

    yyjson_mut_val* caches = yyjson_mut_obj_add_obj(doc, module, "result");

    for (uint32_t i = 0; i < sizeof (result.caches) / sizeof (result.caches[0]) && result.caches[i].length > 0; i++)
    {
        yyjson_mut_val* level = yyjson_mut_obj_add_arr(doc, caches, &"l1\0l2\0l3\0l4\0"[i * 3]);
        FF_LIST_FOR_EACH(FFCPUCache, src, result.caches[i])
        {
            yyjson_mut_val* item = yyjson_mut_arr_add_obj(doc, level);
            yyjson_mut_obj_add_uint(doc, item, "size", src->size);
            yyjson_mut_obj_add_uint(doc, item, "num", src->num);
            const char* typeStr = "unknown";
            switch (src->type)
            {
                case FF_CPU_CACHE_TYPE_DATA: typeStr = "data"; break;
                case FF_CPU_CACHE_TYPE_INSTRUCTION: typeStr = "instruction"; break;
                case FF_CPU_CACHE_TYPE_UNIFIED: typeStr = "unified"; break;
                case FF_CPU_CACHE_TYPE_TRACE: typeStr = "trace"; break;
            }
            yyjson_mut_obj_add_uint(doc, item, "lineSize", src->lineSize);
            yyjson_mut_obj_add_str(doc, item, "type", typeStr);
        }
    }

exit:
    ffListDestroy(&result.caches[0]);
    ffListDestroy(&result.caches[1]);
    ffListDestroy(&result.caches[2]);
    ffListDestroy(&result.caches[3]);
}

void ffPrintCPUCacheHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_CPUCACHE_MODULE_NAME, "{1}", FF_CPUCACHE_NUM_FORMAT_ARGS, ((const char* []) {
        "cpucache type - type",
        "cpucache vendor - vendor",
        "cpucache version - version",
        "cpucache serial number - serial",
    }));
}

void ffInitCPUCacheOptions(FFCPUCacheOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_CPUCACHE_MODULE_NAME,
        "Print CPU cache sizes",
        ffParseCPUCacheCommandOptions,
        ffParseCPUCacheJsonObject,
        ffPrintCPUCache,
        ffGenerateCPUCacheJsonResult,
        ffPrintCPUCacheHelpFormat,
        ffGenerateCPUCacheJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs);
}

void ffDestroyCPUCacheOptions(FFCPUCacheOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

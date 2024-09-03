#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/cpucache/cpucache.h"
#include "modules/cpucache/cpucache.h"
#include "util/stringUtils.h"

#define FF_CPUCACHE_DISPLAY_NAME "CPU Cache"
#define FF_CPUCACHE_NUM_FORMAT_ARGS 2

static void printCPUCacheNormal(const FFCPUCacheResult* result, FFCPUCacheOptions* options)
{
    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY key = ffStrbufCreate();

    char levelStr[4] = "L";
    for (uint32_t i = 0; i < sizeof (result->caches) / sizeof (result->caches[0]) && result->caches[i].length > 0; i++)
    {
        ffStrbufClear(&key);
        levelStr[1] = (char) ('1' + i);
        if (options->moduleArgs.key.length == 0)
            ffStrbufAppendF(&key, "%s (%s)", FF_CPUCACHE_DISPLAY_NAME, levelStr);
        else
        {
            uint32_t index = i + 1;
            FF_PARSE_FORMAT_STRING_CHECKED(&key, &options->moduleArgs.key, 3, ((FFformatarg[]){
                FF_FORMAT_ARG(index, "index"),
                FF_FORMAT_ARG(levelStr, "level"),
                FF_FORMAT_ARG(options->moduleArgs.keyIcon, "icon"),
            }));
        }

        ffStrbufClear(&buffer);

        uint32_t sum = 0;
        FF_LIST_FOR_EACH(FFCPUCache, src, result->caches[i])
        {
            char typeStr = '?';
            switch (src->type)
            {
                case FF_CPU_CACHE_TYPE_DATA: typeStr = 'D'; break;
                case FF_CPU_CACHE_TYPE_INSTRUCTION: typeStr = 'I'; break;
                case FF_CPU_CACHE_TYPE_UNIFIED: typeStr = 'U'; break;
                case FF_CPU_CACHE_TYPE_TRACE: typeStr = 'T'; break;
            }
            if (buffer.length)
                ffStrbufAppendS(&buffer, ", ");
            if (src->num > 1)
                ffStrbufAppendF(&buffer, "%ux", src->num);
            ffParseSize(src->size, &buffer);
            ffStrbufAppendF(&buffer, " (%c)", typeStr);

            sum += src->size * src->num;
        }

        if(options->moduleArgs.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY);
            ffStrbufPutTo(&buffer, stdout);
        }
        else
        {
            FF_STRBUF_AUTO_DESTROY buffer2 = ffStrbufCreate();
            ffParseSize(sum, &buffer2);
            FF_PRINT_FORMAT_CHECKED(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY, FF_CPUCACHE_NUM_FORMAT_ARGS, ((FFformatarg[]) {
                FF_FORMAT_ARG(buffer, "result"),
                FF_FORMAT_ARG(buffer2, "sum"),
            }));
        }
    }
}

static void printCPUCacheCompact(const FFCPUCacheResult* result, FFCPUCacheOptions* options)
{
    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
    uint64_t sum = 0;
    for (uint32_t i = 0; i < sizeof (result->caches) / sizeof (result->caches[0]) && result->caches[i].length > 0; i++)
    {
        if (buffer.length)
            ffStrbufAppendS(&buffer, ", ");
        uint32_t value = 0;
        FF_LIST_FOR_EACH(FFCPUCache, src, result->caches[i])
            value += src->size * src->num;
        ffParseSize(value, &buffer);
        ffStrbufAppendF(&buffer, " (L%u)", i + 1);
        sum += value;
    }

    if (options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_CPUCACHE_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        ffStrbufPutTo(&buffer, stdout);
    }
    else
    {
        FF_STRBUF_AUTO_DESTROY buffer2 = ffStrbufCreate();
        ffParseSize(sum, &buffer2);
        FF_PRINT_FORMAT_CHECKED(FF_CPUCACHE_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_CPUCACHE_NUM_FORMAT_ARGS, ((FFformatarg[]) {
            FF_FORMAT_ARG(buffer, "result"),
            FF_FORMAT_ARG(buffer2, "sum"),
        }));
    }
}

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
        ffPrintError(FF_CPUCACHE_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        goto exit;
    }

    if (!options->compact)
        printCPUCacheNormal(&result, options);
    else
        printCPUCacheCompact(&result, options);

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

    if (ffStrEqualsIgnCase(subKey, "compact"))
    {
        options->compact = ffOptionParseBoolean(value);
        return true;
    }

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

        if (ffStrEqualsIgnCase(key, "compact"))
        {
            options->compact = yyjson_get_bool(val);
            continue;
        }

        ffPrintError(FF_CPUCACHE_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
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
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_CPUCACHE_DISPLAY_NAME, "{1}", FF_CPUCACHE_NUM_FORMAT_ARGS, ((const char* []) {
        "Separate result - result",
        "Sum result - sum",
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
    ffOptionInitModuleArg(&options->moduleArgs, "");

    options->compact = false;
}

void ffDestroyCPUCacheOptions(FFCPUCacheOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

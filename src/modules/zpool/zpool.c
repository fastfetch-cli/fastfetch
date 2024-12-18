#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/percent.h"
#include "detection/zpool/zpool.h"
#include "modules/zpool/zpool.h"
#include "util/stringUtils.h"

static void printZpool(FFZpoolOptions* options, FFZpoolResult* result, uint8_t index)
{
    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
    if (options->moduleArgs.key.length == 0)
    {
        if (result->name.length > 0)
            ffStrbufSetF(&buffer, "%s (%s)", FF_ZPOOL_MODULE_NAME, result->name.chars);
        else
            ffStrbufSetS(&buffer, FF_ZPOOL_MODULE_NAME);
    }
    else
    {
        ffStrbufClear(&buffer);
        FF_PARSE_FORMAT_STRING_CHECKED(&buffer, &options->moduleArgs.key, ((FFformatarg[]) {
            FF_FORMAT_ARG(index, "index"),
            FF_FORMAT_ARG(result->name, "name"),
            FF_FORMAT_ARG(options->moduleArgs.keyIcon, "icon"),
        }));
    }

    FF_STRBUF_AUTO_DESTROY usedPretty = ffStrbufCreate();
    ffParseSize(result->used, &usedPretty);

    FF_STRBUF_AUTO_DESTROY totalPretty = ffStrbufCreate();
    ffParseSize(result->total, &totalPretty);

    double bytesPercentage = result->total > 0 ? (double) result->used / (double) result->total * 100.0 : 0;
    FFPercentageTypeFlags percentType = options->percent.type == 0 ? instance.config.display.percentType : options->percent.type;

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(buffer.chars, index, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY);

        ffStrbufClear(&buffer);
        ffStrbufSetF(&buffer, "%s / %s (", usedPretty.chars, totalPretty.chars);
        ffPercentAppendNum(&buffer, bytesPercentage, options->percent, false, &options->moduleArgs);
        ffStrbufAppendS(&buffer, ", ");
        ffPercentAppendNum(&buffer, result->fragmentation, options->percent, false, &options->moduleArgs);
        ffStrbufAppendF(&buffer, " frag) - %s", result->state.chars);
        ffStrbufPutTo(&buffer, stdout);
    }
    else
    {
        FF_STRBUF_AUTO_DESTROY bytesPercentageNum = ffStrbufCreate();
        if (percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
            ffPercentAppendNum(&bytesPercentageNum, bytesPercentage, options->percent, false, &options->moduleArgs);
        FF_STRBUF_AUTO_DESTROY bytesPercentageBar = ffStrbufCreate();
        if (percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
            ffPercentAppendBar(&bytesPercentageBar, bytesPercentage, options->percent, &options->moduleArgs);

        FF_STRBUF_AUTO_DESTROY fragPercentageNum = ffStrbufCreate();
        if (percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
            ffPercentAppendNum(&fragPercentageNum, result->fragmentation, options->percent, false, &options->moduleArgs);
        FF_STRBUF_AUTO_DESTROY fragPercentageBar = ffStrbufCreate();
        if (percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
            ffPercentAppendBar(&fragPercentageBar, result->fragmentation, options->percent, &options->moduleArgs);

        FF_PRINT_FORMAT_CHECKED(buffer.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY, ((FFformatarg[]) {
            FF_FORMAT_ARG(result->name, "name"),
            FF_FORMAT_ARG(result->state, "state"),
            FF_FORMAT_ARG(usedPretty, "size-used"),
            FF_FORMAT_ARG(totalPretty, "size-total"),
            FF_FORMAT_ARG(bytesPercentageNum, "size-percentage"),
            FF_FORMAT_ARG(fragPercentageNum, "frag-percentage"),
            FF_FORMAT_ARG(bytesPercentageBar, "size-percentage-bar"),
            FF_FORMAT_ARG(fragPercentageBar, "frag-percentage-bar"),
        }));
    }
}

void ffPrintZpool(FFZpoolOptions* options)
{
    FF_LIST_AUTO_DESTROY results = ffListCreate(sizeof(FFZpoolResult));

    const char* error = ffDetectZpool(&results);

    if (error)
    {
        ffPrintError(FF_ZPOOL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return;
    }
    if(results.length == 0)
    {
        ffPrintError(FF_ZPOOL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", "No zpool found");
        return;
    }

    for(uint32_t i = 0; i < results.length; i++)
    {
        FFZpoolResult* result = FF_LIST_GET(FFZpoolResult, results, i);
        uint8_t index = results.length == 1 ? 0 : (uint8_t) (i + 1);
        printZpool(options, result, index);
    }

    FF_LIST_FOR_EACH(FFZpoolResult, result, results)
    {
        ffStrbufDestroy(&result->name);
        ffStrbufDestroy(&result->state);
    }
}

bool ffParseZpoolCommandOptions(FFZpoolOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_ZPOOL_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    if (ffPercentParseCommandOptions(key, subKey, value, &options->percent))
        return true;

    return false;
}

void ffParseZpoolJsonObject(FFZpoolOptions* options, yyjson_val* module)
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

        if (ffPercentParseJsonObject(key, val, &options->percent))
            continue;

        ffPrintError(FF_ZPOOL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateZpoolJsonConfig(FFZpoolOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyZpoolOptions))) FFZpoolOptions defaultOptions;
    ffInitZpoolOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);

    ffPercentGenerateJsonConfig(doc, module, defaultOptions.percent, options->percent);
}

void ffGenerateZpoolJsonResult(FF_MAYBE_UNUSED FFZpoolOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY results = ffListCreate(sizeof(FFZpoolResult));

    const char* error = ffDetectZpool(&results);
    if (error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");

    FF_LIST_FOR_EACH(FFZpoolResult, zpool, results)
    {
        yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);
        yyjson_mut_obj_add_strbuf(doc, obj, "name", &zpool->name);
        yyjson_mut_obj_add_strbuf(doc, obj, "state", &zpool->state);
        yyjson_mut_obj_add_uint(doc, obj, "used", zpool->used);
        yyjson_mut_obj_add_uint(doc, obj, "total", zpool->total);
        yyjson_mut_obj_add_uint(doc, obj, "version", zpool->version);
        yyjson_mut_obj_add_real(doc, obj, "fragmentation", zpool->fragmentation);
    }

    FF_LIST_FOR_EACH(FFZpoolResult, zpool, results)
    {
        ffStrbufDestroy(&zpool->name);
        ffStrbufDestroy(&zpool->state);
    }
}

static FFModuleBaseInfo ffModuleInfo = {
    .name = FF_ZPOOL_MODULE_NAME,
    .description = "Print ZFS storage pools",
    .parseCommandOptions = (void*) ffParseZpoolCommandOptions,
    .parseJsonObject = (void*) ffParseZpoolJsonObject,
    .printModule = (void*) ffPrintZpool,
    .generateJsonResult = (void*) ffGenerateZpoolJsonResult,
    .generateJsonConfig = (void*) ffGenerateZpoolJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Zpool name", "name"},
        {"Zpool state", "state"},
        {"Size used", "used"},
        {"Size total", "total"},
        {"Size percentage num", "used-percentage"},
        {"Fragmentation percentage num", "fragmentation-percentage"},
        {"Size percentage bar", "used-percentage-bar"},
        {"Fragmentation percentage bar", "fragmentation-percentage-bar"},
    }))
};

void ffInitZpoolOptions(FFZpoolOptions* options)
{
    options->moduleInfo = ffModuleInfo;
    ffOptionInitModuleArg(&options->moduleArgs, "󱑛");
    options->percent = (FFPercentageModuleConfig) { 50, 80, 0 };
}

void ffDestroyZpoolOptions(FFZpoolOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

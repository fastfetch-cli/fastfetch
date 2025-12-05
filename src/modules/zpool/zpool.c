#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/percent.h"
#include "common/size.h"
#include "detection/zpool/zpool.h"
#include "modules/zpool/zpool.h"
#include "util/FFstrbuf.h"
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
            FF_FORMAT_ARG(result->guid, "guid"),
            FF_FORMAT_ARG(options->moduleArgs.keyIcon, "icon"),
        }));
    }

    FF_STRBUF_AUTO_DESTROY usedPretty = ffStrbufCreate();
    ffSizeAppendNum(result->used, &usedPretty);

    FF_STRBUF_AUTO_DESTROY allocatedPretty = ffStrbufCreate();
    ffSizeAppendNum(result->allocated, &allocatedPretty);

    FF_STRBUF_AUTO_DESTROY totalPretty = ffStrbufCreate();
    ffSizeAppendNum(result->total, &totalPretty);

    double usedPercentage = result->total > 0 ? (double) result->used / (double) result->total * 100.0 : 0;
    double allocatedPercentage = result->total > 0 ? (double) result->allocated / (double) result->total * 100.0 : 0;
    FFPercentageTypeFlags percentType = options->percent.type == 0 ? instance.config.display.percentType : options->percent.type;

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(buffer.chars, index, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY);

        ffStrbufClear(&buffer);
        ffStrbufSetF(&buffer, "%s / %s (", usedPretty.chars, totalPretty.chars);
        ffPercentAppendNum(&buffer, usedPercentage, options->percent, false, &options->moduleArgs);
        ffStrbufAppendS(&buffer, ", ");
        ffPercentAppendNum(&buffer, allocatedPercentage, options->percent, false, &options->moduleArgs);
        ffStrbufAppendS(&buffer, " allocated, ");
        ffPercentAppendNum(&buffer, result->fragmentation, options->percent, false, &options->moduleArgs);
        ffStrbufAppendF(&buffer, " frag) - %s", result->state.chars);
        if (result->readOnly)
            ffStrbufAppendS(&buffer, " [Read-only]");
        ffStrbufPutTo(&buffer, stdout);
    }
    else
    {
        FF_STRBUF_AUTO_DESTROY usedPercentageNum = ffStrbufCreate();
        if (percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
            ffPercentAppendNum(&usedPercentageNum, usedPercentage, options->percent, false, &options->moduleArgs);
        FF_STRBUF_AUTO_DESTROY usedPercentageBar = ffStrbufCreate();
        if (percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
            ffPercentAppendBar(&usedPercentageBar, usedPercentage, options->percent, &options->moduleArgs);

        FF_STRBUF_AUTO_DESTROY allocatedPercentageNum = ffStrbufCreate();
        if (percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
            ffPercentAppendNum(&allocatedPercentageNum, allocatedPercentage, options->percent, false, &options->moduleArgs);
        FF_STRBUF_AUTO_DESTROY allocatedPercentageBar = ffStrbufCreate();
        if (percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
            ffPercentAppendBar(&allocatedPercentageBar, allocatedPercentage, options->percent, &options->moduleArgs);

        FF_STRBUF_AUTO_DESTROY fragPercentageNum = ffStrbufCreate();
        if (percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
            ffPercentAppendNum(&fragPercentageNum, result->fragmentation, options->percent, false, &options->moduleArgs);
        FF_STRBUF_AUTO_DESTROY fragPercentageBar = ffStrbufCreate();
        if (percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
            ffPercentAppendBar(&fragPercentageBar, result->fragmentation, options->percent, &options->moduleArgs);

        FF_PRINT_FORMAT_CHECKED(buffer.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY, ((FFformatarg[]) {
            FF_FORMAT_ARG(result->name, "name"),
            FF_FORMAT_ARG(result->guid, "guid"),
            FF_FORMAT_ARG(result->state, "state"),
            FF_FORMAT_ARG(usedPretty, "size-used"),
            FF_FORMAT_ARG(allocatedPretty, "size-allocated"),
            FF_FORMAT_ARG(totalPretty, "size-total"),
            FF_FORMAT_ARG(usedPercentageNum, "used-percentage"),
            FF_FORMAT_ARG(allocatedPercentageNum, "allocated-percentage"),
            FF_FORMAT_ARG(fragPercentageNum, "frag-percentage"),
            FF_FORMAT_ARG(usedPercentageBar, "used-percentage-bar"),
            FF_FORMAT_ARG(allocatedPercentageBar, "allocated-percentage-bar"),
            FF_FORMAT_ARG(fragPercentageBar, "frag-percentage-bar"),
            FF_FORMAT_ARG(result->readOnly, "is-readonly"),
        }));
    }
}

bool ffPrintZpool(FFZpoolOptions* options)
{
    FF_LIST_AUTO_DESTROY results = ffListCreate(sizeof(FFZpoolResult));

    const char* error = ffDetectZpool(&results);

    if (error)
    {
        ffPrintError(FF_ZPOOL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return false;
    }
    if(results.length == 0)
    {
        ffPrintError(FF_ZPOOL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", "No zpool found");
        return false;
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
    return true;
}

void ffParseZpoolJsonObject(FFZpoolOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        if (ffPercentParseJsonObject(key, val, &options->percent))
            continue;

        ffPrintError(FF_ZPOOL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateZpoolJsonConfig(FFZpoolOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);

    ffPercentGenerateJsonConfig(doc, module, options->percent);
}

bool ffGenerateZpoolJsonResult(FF_MAYBE_UNUSED FFZpoolOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY results = ffListCreate(sizeof(FFZpoolResult));

    const char* error = ffDetectZpool(&results);
    if (error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return false;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");

    FF_LIST_FOR_EACH(FFZpoolResult, zpool, results)
    {
        yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);
        yyjson_mut_obj_add_strbuf(doc, obj, "name", &zpool->name);
        yyjson_mut_obj_add_strbuf(doc, obj, "state", &zpool->state);
        yyjson_mut_obj_add_uint(doc, obj, "guid", zpool->guid);
        yyjson_mut_obj_add_uint(doc, obj, "used", zpool->used);
        yyjson_mut_obj_add_uint(doc, obj, "allocated", zpool->allocated);
        yyjson_mut_obj_add_uint(doc, obj, "total", zpool->total);
        if (zpool->fragmentation != -DBL_MAX)
            yyjson_mut_obj_add_real(doc, obj, "fragmentation", zpool->fragmentation);
        else
            yyjson_mut_obj_add_null(doc, obj, "fragmentation");
        yyjson_mut_obj_add_bool(doc, obj, "readOnly", zpool->readOnly);
    }

    FF_LIST_FOR_EACH(FFZpoolResult, zpool, results)
    {
        ffStrbufDestroy(&zpool->name);
        ffStrbufDestroy(&zpool->state);
    }
    return true;
}

void ffInitZpoolOptions(FFZpoolOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "󱑛");
    options->percent = (FFPercentageModuleConfig) { 50, 80, 0 };
}

void ffDestroyZpoolOptions(FFZpoolOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffZpoolModuleInfo = {
    .name = FF_ZPOOL_MODULE_NAME,
    .description = "Print ZFS storage pools",
    .initOptions = (void*) ffInitZpoolOptions,
    .destroyOptions = (void*) ffDestroyZpoolOptions,
    .parseJsonObject = (void*) ffParseZpoolJsonObject,
    .printModule = (void*) ffPrintZpool,
    .generateJsonResult = (void*) ffGenerateZpoolJsonResult,
    .generateJsonConfig = (void*) ffGenerateZpoolJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Zpool name", "name"},
        {"Zpool guid", "guid"},
        {"Zpool state", "state"},
        {"Size used", "used"},
        {"Size allocated", "allocated"},
        {"Size total", "total"},
        {"Size used percentage num", "used-percentage"},
        {"Size allocated percentage num", "allocated-percentage"},
        {"Fragmentation percentage num", "fragmentation-percentage"},
        {"Size used percentage bar", "used-percentage-bar"},
        {"Size allocated percentage bar", "allocated-percentage-bar"},
        {"Fragmentation percentage bar", "fragmentation-percentage-bar"},
    }))
};

#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/percent.h"
#include "common/size.h"
#include "detection/swap/swap.h"
#include "modules/swap/swap.h"
#include "util/stringUtils.h"

void printSwap(FFSwapOptions* options, uint8_t index, FFSwapResult* storage)
{
    FF_STRBUF_AUTO_DESTROY key = ffStrbufCreate();

    if (options->moduleArgs.key.length == 0)
    {
        if (storage->name.length > 0)
            ffStrbufSetF(&key, "%s (%s)", FF_SWAP_MODULE_NAME, storage->name.chars);
        else
            ffStrbufSetS(&key, FF_SWAP_MODULE_NAME);
    }
    else
    {
        ffStrbufClear(&key);
        FF_PARSE_FORMAT_STRING_CHECKED(&key, &options->moduleArgs.key, ((FFformatarg[]) {
            FF_FORMAT_ARG(index, "index"),
            FF_FORMAT_ARG(storage->name, "name"),
            FF_FORMAT_ARG(options->moduleArgs.keyIcon, "icon"),
        }));
    }

    FF_STRBUF_AUTO_DESTROY usedPretty = ffStrbufCreate();
    ffSizeAppendNum(storage->bytesUsed, &usedPretty);

    FF_STRBUF_AUTO_DESTROY totalPretty = ffStrbufCreate();
    ffSizeAppendNum(storage->bytesTotal, &totalPretty);

    double percentage = storage->bytesTotal == 0
        ? 0
        : (double) storage->bytesUsed / (double) storage->bytesTotal * 100.0;

    FFPercentageTypeFlags percentType = options->percent.type == 0 ? instance.config.display.percentType : options->percent.type;
    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY);
        FF_STRBUF_AUTO_DESTROY str = ffStrbufCreate();

        if (storage->bytesTotal == 0)
        {
            if(percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
            {
                ffPercentAppendBar(&str, 0, options->percent, &options->moduleArgs);
                ffStrbufAppendC(&str, ' ');
            }
            if(!(percentType & FF_PERCENTAGE_TYPE_HIDE_OTHERS_BIT))
                ffStrbufAppendS(&str, "Disabled");
            else
                ffPercentAppendNum(&str, 0, options->percent, str.length > 0, &options->moduleArgs);
        }
        else
        {
            if(percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
            {
                ffPercentAppendBar(&str, percentage, options->percent, &options->moduleArgs);
                ffStrbufAppendC(&str, ' ');
            }

            if(!(percentType & FF_PERCENTAGE_TYPE_HIDE_OTHERS_BIT))
                ffStrbufAppendF(&str, "%s / %s ", usedPretty.chars, totalPretty.chars);

            if(percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
                ffPercentAppendNum(&str, percentage, options->percent, str.length > 0, &options->moduleArgs);
        }

        ffStrbufTrimRight(&str, ' ');
        ffStrbufPutTo(&str, stdout);
    }
    else
    {
        FF_STRBUF_AUTO_DESTROY percentageNum = ffStrbufCreate();
        if (percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
            ffPercentAppendNum(&percentageNum, percentage, options->percent, false, &options->moduleArgs);
        FF_STRBUF_AUTO_DESTROY percentageBar = ffStrbufCreate();
        if (percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
            ffPercentAppendBar(&percentageBar, percentage, options->percent, &options->moduleArgs);
        FF_PRINT_FORMAT_CHECKED(key.chars, index, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY, ((FFformatarg[]){
            FF_FORMAT_ARG(usedPretty, "used"),
            FF_FORMAT_ARG(totalPretty, "total"),
            FF_FORMAT_ARG(percentageNum, "percentage"),
            FF_FORMAT_ARG(percentageBar, "percentage-bar"),
            FF_FORMAT_ARG(storage->name, "name"),
        }));
    }
}

bool ffPrintSwap(FFSwapOptions* options)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFSwapResult));
    const char* error = ffDetectSwap(&result);

    if(error)
    {
        ffPrintError(FF_SWAP_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return false;
    }

    if (options->separate)
    {
        uint8_t index = 0;
        FF_LIST_FOR_EACH(FFSwapResult, storage, result)
        {
            ++index;
            printSwap(options, index, storage);
        }
    }
    else
    {
        FFSwapResult total = {
            .name = ffStrbufCreate(),
        };
        FF_LIST_FOR_EACH(FFSwapResult, storage, result)
        {
            total.bytesUsed += storage->bytesUsed;
            total.bytesTotal += storage->bytesTotal;
        }
        printSwap(options, 0, &total);
        ffStrbufDestroy(&total.name);
    }

    FF_LIST_FOR_EACH(FFSwapResult, storage, result)
    {
        ffStrbufDestroy(&storage->name);
    }

    return true;
}

void ffParseSwapJsonObject(FFSwapOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        if (ffPercentParseJsonObject(key, val, &options->percent))
            continue;

        if (unsafe_yyjson_equals_str(key, "separate"))
        {
            options->separate = yyjson_get_bool(val);
            continue;
        }

        ffPrintError(FF_SWAP_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateSwapJsonConfig(FFSwapOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffPercentGenerateJsonConfig(doc, module, options->percent);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
    yyjson_mut_obj_add_bool(doc, module, "separate", options->separate);
}

bool ffGenerateSwapJsonResult(FF_MAYBE_UNUSED FFSwapOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFSwapResult));
    const char* error = ffDetectSwap(&result);

    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return false;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");
    FF_LIST_FOR_EACH(FFSwapResult, storage, result)
    {
        yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);
        yyjson_mut_obj_add_strbuf(doc, obj, "name", &storage->name);
        yyjson_mut_obj_add_uint(doc, obj, "used", storage->bytesUsed);
        yyjson_mut_obj_add_uint(doc, obj, "total", storage->bytesTotal);
    }

    FF_LIST_FOR_EACH(FFSwapResult, storage, result)
    {
        ffStrbufDestroy(&storage->name);
    }

    return true;
}

void ffInitSwapOptions(FFSwapOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "󰓡");
    options->percent = (FFPercentageModuleConfig) { 50, 80, 0 };
    options->separate = false;
}

void ffDestroySwapOptions(FFSwapOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffSwapModuleInfo = {
    .name = FF_SWAP_MODULE_NAME,
    .description = "Print swap (paging file) space usage",
    .initOptions = (void*) ffInitSwapOptions,
    .destroyOptions = (void*) ffDestroySwapOptions,
    .parseJsonObject = (void*) ffParseSwapJsonObject,
    .printModule = (void*) ffPrintSwap,
    .generateJsonResult = (void*) ffGenerateSwapJsonResult,
    .generateJsonConfig = (void*) ffGenerateSwapJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Used size", "used"},
        {"Total size", "total"},
        {"Percentage used (num)", "percentage"},
        {"Percentage used (bar)", "percentage-bar"},
        {"Name", "name"},
    }))
};

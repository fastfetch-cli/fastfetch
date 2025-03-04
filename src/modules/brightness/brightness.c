#include "common/percent.h"
#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/brightness/brightness.h"
#include "modules/brightness/brightness.h"
#include "util/stringUtils.h"

void ffPrintBrightness(FFBrightnessOptions* options)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFBrightnessResult));

    const char* error = ffDetectBrightness(options, &result);

    if(error)
    {
        ffPrintError(FF_BRIGHTNESS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return;
    }

    if(result.length == 0)
    {
        ffPrintError(FF_BRIGHTNESS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "No result is detected.");
        return;
    }

    FFPercentageTypeFlags percentType = options->percent.type == 0 ? instance.config.display.percentType : options->percent.type;

    if (options->compact)
    {
        FF_STRBUF_AUTO_DESTROY str = ffStrbufCreate();

        FF_LIST_FOR_EACH(FFBrightnessResult, item, result)
        {
            if(str.length > 0)
                ffStrbufAppendC(&str, ' ');

            const double percent = (item->current - item->min) / (item->max - item->min) * 100;
            ffPercentAppendNum(&str, percent, options->percent, false, &options->moduleArgs);
        }

        ffPrintLogoAndKey(FF_BRIGHTNESS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        ffStrbufPutTo(&str, stdout);
        return;
    }

    FF_STRBUF_AUTO_DESTROY key = ffStrbufCreate();

    uint32_t index = 0;
    FF_LIST_FOR_EACH(FFBrightnessResult, item, result)
    {
        if (options->moduleArgs.key.length == 0)
        {
            ffStrbufAppendF(&key, "%s (%s)", FF_BRIGHTNESS_MODULE_NAME, item->name.chars);
        }
        else
        {
            uint32_t moduleIndex = result.length == 1 ? 0 : index + 1;
            FF_PARSE_FORMAT_STRING_CHECKED(&key, &options->moduleArgs.key, ((FFformatarg[]) {
                FF_FORMAT_ARG(moduleIndex, "index"),
                FF_FORMAT_ARG(item->name, "name"),
                FF_FORMAT_ARG(options->moduleArgs.keyIcon, "icon"),
            }));
        }

        const double percent = (item->current - item->min) / (item->max - item->min) * 100;

        if (options->moduleArgs.outputFormat.length == 0)
        {
            FF_STRBUF_AUTO_DESTROY str = ffStrbufCreate();
            ffPrintLogoAndKey(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY);

            if (percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
            {
                ffPercentAppendBar(&str, percent, options->percent, &options->moduleArgs);
            }

            if (percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
            {
                if(str.length > 0)
                    ffStrbufAppendC(&str, ' ');

                ffPercentAppendNum(&str, percent, options->percent, str.length > 0, &options->moduleArgs);
            }

            ffStrbufAppendS(&str, item->builtin ? " [Built-in]" : " [External]");

            ffStrbufPutTo(&str, stdout);
        }
        else
        {
            FF_STRBUF_AUTO_DESTROY valueNum = ffStrbufCreate();
            if (percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
                ffPercentAppendNum(&valueNum, percent, options->percent, false, &options->moduleArgs);
            FF_STRBUF_AUTO_DESTROY valueBar = ffStrbufCreate();
            if (percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
                ffPercentAppendBar(&valueBar, percent, options->percent, &options->moduleArgs);

            FF_PRINT_FORMAT_CHECKED(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY, ((FFformatarg[]) {
                FF_FORMAT_ARG(valueNum, "percentage"),
                FF_FORMAT_ARG(item->name, "name"),
                FF_FORMAT_ARG(item->max, "max"),
                FF_FORMAT_ARG(item->min, "min"),
                FF_FORMAT_ARG(item->current, "current"),
                FF_FORMAT_ARG(valueBar, "percentage-bar"),
                FF_FORMAT_ARG(item->builtin, "is-builtin"),
            }));
        }

        ffStrbufClear(&key);
        ffStrbufDestroy(&item->name);
        ++index;
    }
}

bool ffParseBrightnessCommandOptions(FFBrightnessOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_BRIGHTNESS_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    if (ffStrEqualsIgnCase(subKey, "ddcci-sleep"))
    {
        options->ddcciSleep = ffOptionParseUInt32(key, value);
        return true;
    }

    if (ffStrEqualsIgnCase(subKey, "compact"))
    {
        options->compact = ffOptionParseBoolean(value);
        return true;
    }

    if (ffPercentParseCommandOptions(key, subKey, value, &options->percent))
        return true;

    return false;
}

void ffParseBrightnessJsonObject(FFBrightnessOptions* options, yyjson_val* module)
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

        if (ffStrEqualsIgnCase(key, "ddcciSleep"))
        {
            options->ddcciSleep = (uint32_t) yyjson_get_uint(val);
            continue;
        }

        if (ffStrEqualsIgnCase(key, "compact"))
        {
            options->compact = (uint32_t) yyjson_get_bool(val);
            continue;
        }

        if (ffPercentParseJsonObject(key, val, &options->percent))
            continue;

        ffPrintError(FF_BRIGHTNESS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateBrightnessJsonConfig(FFBrightnessOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyBrightnessOptions))) FFBrightnessOptions defaultOptions;
    ffInitBrightnessOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);

    if (defaultOptions.ddcciSleep != options->ddcciSleep)
        yyjson_mut_obj_add_uint(doc, module, "ddcciSleep", options->ddcciSleep);

    ffPercentGenerateJsonConfig(doc, module, defaultOptions.percent, options->percent);

    if (defaultOptions.compact != options->compact)
        yyjson_mut_obj_add_bool(doc, module, "compact", options->compact);
}

void ffGenerateBrightnessJsonResult(FF_MAYBE_UNUSED FFBrightnessOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFBrightnessResult));

    const char* error = ffDetectBrightness(options, &result);

    if (error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return;
    }

    yyjson_mut_val* arr = yyjson_mut_arr(doc);
    yyjson_mut_obj_add_val(doc, module, "result", arr);

    FF_LIST_FOR_EACH(FFBrightnessResult, item, result)
    {
        yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);
        yyjson_mut_obj_add_strbuf(doc, obj, "name", &item->name);
        yyjson_mut_obj_add_real(doc, obj, "max", item->max);
        yyjson_mut_obj_add_real(doc, obj, "min", item->min);
        yyjson_mut_obj_add_real(doc, obj, "current", item->current);
        yyjson_mut_obj_add_bool(doc, obj, "builtin", item->builtin);
    }

    FF_LIST_FOR_EACH(FFBrightnessResult, item, result)
    {
        ffStrbufDestroy(&item->name);
    }
}

static FFModuleBaseInfo ffModuleInfo = {
    .name = FF_BRIGHTNESS_MODULE_NAME,
    .description = "Print current brightness level of your monitors",
    .parseCommandOptions = (void*) ffParseBrightnessCommandOptions,
    .parseJsonObject = (void*) ffParseBrightnessJsonObject,
    .printModule = (void*) ffPrintBrightness,
    .generateJsonResult = (void*) ffGenerateBrightnessJsonResult,
    .generateJsonConfig = (void*) ffGenerateBrightnessJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Screen brightness (percentage num)", "percentage"},
        {"Screen name", "name"},
        {"Maximum brightness value", "max"},
        {"Minimum brightness value", "min"},
        {"Current brightness value", "current"},
        {"Screen brightness (percentage bar)", "percentage-bar"},
        {"Is built-in screen", "is-builtin"},
    }))
};

void ffInitBrightnessOptions(FFBrightnessOptions* options)
{
    options->moduleInfo = ffModuleInfo;
    ffOptionInitModuleArg(&options->moduleArgs, "󰯪");

    options->ddcciSleep = 10;
    options->percent = (FFPercentageModuleConfig) { 100, 100, 0 };
    options->compact = false;
}

void ffDestroyBrightnessOptions(FFBrightnessOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

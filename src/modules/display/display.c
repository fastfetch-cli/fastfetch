#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/displayserver/displayserver.h"
#include "modules/display/display.h"
#include "util/stringUtils.h"

#define FF_DISPLAY_NUM_FORMAT_ARGS 9

static int sortByNameAsc(FFDisplayResult* a, FFDisplayResult* b)
{
    return ffStrbufComp(&a->name, &b->name);
}

static int sortByNameDesc(FFDisplayResult* a, FFDisplayResult* b)
{
    return -ffStrbufComp(&a->name, &b->name);
}

void ffPrintDisplay(FFDisplayOptions* options)
{
    const FFDisplayServerResult* dsResult = ffConnectDisplayServer();

    if(dsResult->displays.length == 0)
    {
        ffPrintError(FF_DISPLAY_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Couldn't detect display");
        return;
    }

    if (options->order != FF_DISPLAY_ORDER_NONE)
    {
        ffListSort((FFlist*) &dsResult->displays, (void*) (options->order == FF_DISPLAY_ORDER_ASC ? sortByNameAsc : sortByNameDesc));
    }

    if (options->compactType != FF_DISPLAY_COMPACT_TYPE_NONE)
    {
        ffPrintLogoAndKey(FF_DISPLAY_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

        FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
        FF_LIST_FOR_EACH(FFDisplayResult, result, dsResult->displays)
        {
            if (options->compactType & FF_DISPLAY_COMPACT_TYPE_ORIGINAL_BIT)
            {
                ffStrbufAppendF(&buffer, "%ix%i", result->width, result->height);
            }
            else
            {
                ffStrbufAppendF(&buffer, "%ix%i", result->scaledWidth, result->scaledHeight);
            }

            if (options->compactType & FF_DISPLAY_COMPACT_TYPE_REFRESH_RATE_BIT)
            {
                if (result->refreshRate > 0)
                {
                    if (options->preciseRefreshRate)
                        ffStrbufAppendF(&buffer, " @ %gHz", result->refreshRate);
                    else
                        ffStrbufAppendF(&buffer, " @ %iHz", (uint32_t) (result->refreshRate + 0.5));
                }
                ffStrbufAppendS(&buffer, ", ");
            }
            else
            {
                ffStrbufAppendC(&buffer, ' ');
            }
        }
        ffStrbufTrimRight(&buffer, ' ');
        ffStrbufTrimRight(&buffer, ',');
        ffStrbufPutTo(&buffer, stdout);
        return;
    }

    FF_STRBUF_AUTO_DESTROY key = ffStrbufCreate();

    for(uint32_t i = 0; i < dsResult->displays.length; i++)
    {
        FFDisplayResult* result = ffListGet(&dsResult->displays, i);
        uint32_t moduleIndex = dsResult->displays.length == 1 ? 0 : i + 1;
        const char* displayType = result->type == FF_DISPLAY_TYPE_UNKNOWN ? NULL : result->type == FF_DISPLAY_TYPE_BUILTIN ? "built-in" : "external";

        ffStrbufClear(&key);
        if(options->moduleArgs.key.length == 0)
        {
            if (result->name.length)
                ffStrbufAppendF(&key, "%s (%s)", FF_DISPLAY_MODULE_NAME, result->name.chars);
            else if (moduleIndex > 0)
                ffStrbufAppendF(&key, "%s (%d)", FF_DISPLAY_MODULE_NAME, moduleIndex);
            else
                ffStrbufAppendS(&key, FF_DISPLAY_MODULE_NAME);
        }
        else
        {
            FF_PARSE_FORMAT_STRING_CHECKED(&key, &options->moduleArgs.key, 3, ((FFformatarg[]){
                {FF_FORMAT_ARG_TYPE_UINT, &moduleIndex},
                {FF_FORMAT_ARG_TYPE_STRBUF, &result->name},
                {FF_FORMAT_ARG_TYPE_STRING, displayType},
            }));
        }

        if(options->moduleArgs.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY);

            printf("%ix%i", result->width, result->height);

            if(result->refreshRate > 0)
            {
                if(options->preciseRefreshRate)
                    printf(" @ %gHz", ((int) (result->refreshRate * 1000 + 0.5)) / 1000.0);
                else
                    printf(" @ %iHz", (uint32_t) (result->refreshRate + 0.5));
            }

            if(
                result->scaledWidth > 0 && result->scaledWidth != result->width &&
                result->scaledHeight > 0 && result->scaledHeight != result->height)
                printf(" (as %ix%i)", result->scaledWidth, result->scaledHeight);

            if(result->type != FF_DISPLAY_TYPE_UNKNOWN)
                fputs(result->type == FF_DISPLAY_TYPE_BUILTIN ? " [Built-in]" : " [External]", stdout);

            if(moduleIndex > 0 && result->primary)
                printf(" *");

            putchar('\n');
        }
        else
        {
            FF_PRINT_FORMAT_CHECKED(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY, FF_DISPLAY_NUM_FORMAT_ARGS, ((FFformatarg[]) {
                {FF_FORMAT_ARG_TYPE_UINT, &result->width},
                {FF_FORMAT_ARG_TYPE_UINT, &result->height},
                {FF_FORMAT_ARG_TYPE_DOUBLE, &result->refreshRate},
                {FF_FORMAT_ARG_TYPE_UINT, &result->scaledWidth},
                {FF_FORMAT_ARG_TYPE_UINT, &result->scaledHeight},
                {FF_FORMAT_ARG_TYPE_STRBUF, &result->name},
                {FF_FORMAT_ARG_TYPE_STRING, displayType},
                {FF_FORMAT_ARG_TYPE_UINT, &result->rotation},
                {FF_FORMAT_ARG_TYPE_BOOL, &result->primary},
            }));
        }
    }
}

bool ffParseDisplayCommandOptions(FFDisplayOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_DISPLAY_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    if (ffStrEqualsIgnCase(subKey, "compact-type"))
    {
        options->compactType = (FFDisplayCompactType) ffOptionParseEnum(key, value, (FFKeyValuePair[]) {
            { "none", FF_DISPLAY_COMPACT_TYPE_NONE },
            { "original", FF_DISPLAY_COMPACT_TYPE_ORIGINAL_BIT },
            { "scaled", FF_DISPLAY_COMPACT_TYPE_SCALED_BIT },
            { "original-with-refresh-rate", FF_DISPLAY_COMPACT_TYPE_ORIGINAL_BIT | FF_DISPLAY_COMPACT_TYPE_REFRESH_RATE_BIT },
            { "scaled-with-refresh-rate", FF_DISPLAY_COMPACT_TYPE_SCALED_BIT | FF_DISPLAY_COMPACT_TYPE_REFRESH_RATE_BIT },
            {},
        });
        return true;
    }

    if (ffStrEqualsIgnCase(subKey, "precise-refresh-rate"))
    {
        options->preciseRefreshRate = ffOptionParseBoolean(value);
        return true;
    }

    if (ffStrEqualsIgnCase(subKey, "order"))
    {
        options->order = (FFDisplayOrder) ffOptionParseEnum(key, value, (FFKeyValuePair[]) {
            { "asc", FF_DISPLAY_ORDER_ASC },
            { "desc", FF_DISPLAY_ORDER_DESC },
            { "none", FF_DISPLAY_ORDER_NONE },
            {},
        });
        return true;
    }

    return false;
}

void ffParseDisplayJsonObject(FFDisplayOptions* options, yyjson_val* module)
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

        if (ffStrEqualsIgnCase(key, "compactType"))
        {
            int value;
            const char* error = ffJsonConfigParseEnum(val, &value, (FFKeyValuePair[]) {
                { "none", FF_DISPLAY_COMPACT_TYPE_NONE },
                { "original", FF_DISPLAY_COMPACT_TYPE_ORIGINAL_BIT },
                { "scaled", FF_DISPLAY_COMPACT_TYPE_SCALED_BIT },
                { "original-with-refresh-rate", FF_DISPLAY_COMPACT_TYPE_ORIGINAL_BIT | FF_DISPLAY_COMPACT_TYPE_REFRESH_RATE_BIT },
                { "scaled-with-refresh-rate", FF_DISPLAY_COMPACT_TYPE_SCALED_BIT | FF_DISPLAY_COMPACT_TYPE_REFRESH_RATE_BIT },
                {},
            });
            if (error)
                ffPrintError(FF_DISPLAY_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Invalid %s value: %s", key, error);
            else
                options->compactType = (FFDisplayCompactType) value;
            continue;
        }

        if (ffStrEqualsIgnCase(key, "preciseRefreshRate"))
        {
            options->preciseRefreshRate = yyjson_get_bool(val);
            continue;
        }

        if (ffStrEqualsIgnCase(key, "order"))
        {
            int value;
            const char* error = ffJsonConfigParseEnum(val, &value, (FFKeyValuePair[]) {
                { "asc", FF_DISPLAY_ORDER_ASC },
                { "desc", FF_DISPLAY_ORDER_DESC },
                { "none", FF_DISPLAY_ORDER_NONE },
                {},
            });
            if (error)
                ffPrintError(FF_DISPLAY_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Invalid %s value: %s", key, error);
            else
                options->order = (FFDisplayOrder) value;
            continue;
        }

        ffPrintError(FF_DISPLAY_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateDisplayJsonConfig(FFDisplayOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyDisplayOptions))) FFDisplayOptions defaultOptions;
    ffInitDisplayOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);

    if (options->compactType != defaultOptions.compactType)
    {
        switch ((int) options->compactType)
        {
            case FF_DISPLAY_COMPACT_TYPE_NONE:
                yyjson_mut_obj_add_str(doc, module, "compactType", "none");
                break;
            case FF_DISPLAY_COMPACT_TYPE_ORIGINAL_BIT:
                yyjson_mut_obj_add_str(doc, module, "compactType", "original");
                break;
            case FF_DISPLAY_COMPACT_TYPE_SCALED_BIT:
                yyjson_mut_obj_add_str(doc, module, "compactType", "scaled");
                break;
            case FF_DISPLAY_COMPACT_TYPE_ORIGINAL_BIT | FF_DISPLAY_COMPACT_TYPE_REFRESH_RATE_BIT:
                yyjson_mut_obj_add_str(doc, module, "compactType", "original-with-refresh-rate");
                break;
            case FF_DISPLAY_COMPACT_TYPE_SCALED_BIT | FF_DISPLAY_COMPACT_TYPE_REFRESH_RATE_BIT:
                yyjson_mut_obj_add_str(doc, module, "compactType", "scaled-with-refresh-rate");
                break;
        }
    }

    if (options->preciseRefreshRate != defaultOptions.preciseRefreshRate)
        yyjson_mut_obj_add_bool(doc, module, "preciseRefreshRate", options->preciseRefreshRate);
}

void ffGenerateDisplayJsonResult(FF_MAYBE_UNUSED FFDisplayOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    const FFDisplayServerResult* dsResult = ffConnectDisplayServer();

    if(dsResult->displays.length == 0)
    {
        yyjson_mut_obj_add_str(doc, module, "error", "Couldn't detect display");
        return;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");
    FF_LIST_FOR_EACH(FFDisplayResult, item, dsResult->displays)
    {
        yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);
        yyjson_mut_obj_add_uint(doc, obj, "width", item->width);
        yyjson_mut_obj_add_uint(doc, obj, "height", item->height);
        yyjson_mut_obj_add_uint(doc, obj, "id", item->id);
        yyjson_mut_obj_add_strbuf(doc, obj, "name", &item->name);
        yyjson_mut_obj_add_bool(doc, obj, "primary", item->primary);
        yyjson_mut_obj_add_real(doc, obj, "refreshRate", item->refreshRate);
        yyjson_mut_obj_add_uint(doc, obj, "rotation", item->rotation);
        yyjson_mut_obj_add_uint(doc, obj, "scaledHeight", item->scaledHeight);
        yyjson_mut_obj_add_uint(doc, obj, "scaledWidth", item->scaledWidth);
        switch (item->type)
        {
            case FF_DISPLAY_TYPE_BUILTIN:
                yyjson_mut_obj_add_str(doc, obj, "type", "Builtin");
                break;
            case FF_DISPLAY_TYPE_EXTERNAL:
                yyjson_mut_obj_add_str(doc, obj, "type", "External");
                break;
            default:
                yyjson_mut_obj_add_str(doc, obj, "type", "Unknown");
                break;
        }
    }
}

void ffPrintDisplayHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_DISPLAY_MODULE_NAME, "{1}x{2} @ {3}Hz (as {4}x{5}) [{7}]", FF_DISPLAY_NUM_FORMAT_ARGS, ((const char* []) {
        "Screen width (in pixels)",
        "Screen height (in pixels)",
        "Screen refresh rate (in Hz)",
        "Screen scaled width (in pixels)",
        "Screen scaled height (in pixels)",
        "Screen name",
        "Screen type (builtin, external or unknown)",
        "Screen rotation (in degrees)",
        "True if being the primary screen",
    }));
}

void ffInitDisplayOptions(FFDisplayOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_DISPLAY_MODULE_NAME,
        "Print resolutions, refresh rates, etc",
        ffParseDisplayCommandOptions,
        ffParseDisplayJsonObject,
        ffPrintDisplay,
        ffGenerateDisplayJsonResult,
        ffPrintDisplayHelpFormat,
        ffGenerateDisplayJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs);
    options->compactType = FF_DISPLAY_COMPACT_TYPE_NONE;
    options->preciseRefreshRate = false;
}

void ffDestroyDisplayOptions(FFDisplayOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

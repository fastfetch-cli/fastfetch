#include "common/printing.h"
#include "common/jsonconfig.h"
#include "util/textModifier.h"
#include "modules/colors/colors.h"
#include "util/stringUtils.h"

static inline uint8_t min(uint8_t a, uint8_t b)
{
    return a < b ? a : b;
}

static inline uint8_t max(uint8_t a, uint8_t b)
{
    return a > b ? a : b;
}

void ffPrintColors(FFColorsOptions* options)
{
    bool flag = false;

    FF_STRBUF_AUTO_DESTROY result = ffStrbufCreateA(128);

    if (options->symbol == FF_COLORS_SYMBOL_BLOCK)
    {
        // 3%d: Set the foreground color
        for(uint8_t i = options->block.range[0]; i <= min(options->block.range[1], 7); i++)
        {
            if (!instance.config.display.pipe)
                ffStrbufAppendF(&result, "\e[3%dm", i);
            for (uint8_t j = 0; j < options->block.width; j++)
                ffStrbufAppendS(&result, "█");
        }
        if (result.length > 0)
        {
            ffPrintLogoAndKey(FF_COLORS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
            flag = true;

            if (options->paddingLeft > 0)
                ffPrintCharTimes(' ', options->paddingLeft);

            if (!instance.config.display.pipe)
                ffStrbufAppendS(&result, FASTFETCH_TEXT_MODIFIER_RESET);
            ffStrbufPutTo(&result, stdout);
            ffStrbufClear(&result);
        }

        // 1: Set everything to bolt. This causes normal colors on some systems to be bright.
        // 9%d: Set the foreground to the bright color
        for(uint8_t i = max(options->block.range[0], 8); i <= options->block.range[1]; i++)
        {
            if(!instance.config.display.pipe)
                ffStrbufAppendF(&result, "\e[1;9%dm", i - 8);
            for (uint8_t j = 0; j < options->block.width; j++)
                ffStrbufAppendS(&result, "█");
        }
    }
    else
    {
        const char* symbol;
        switch (options->symbol)
        {
            case FF_COLORS_SYMBOL_CIRCLE: symbol = "● "; break;
            case FF_COLORS_SYMBOL_DIAMOND: symbol = "◆ "; break;
            case FF_COLORS_SYMBOL_TRIANGLE: symbol = "▲ "; break;
            case FF_COLORS_SYMBOL_SQUARE: symbol = "■ "; break;
            case FF_COLORS_SYMBOL_STAR: symbol = "★ "; break;
            default: symbol = "███ "; break;
        }
        for (int i = 8; i >= 1; --i)
        {
            if (!instance.config.display.pipe)
                ffStrbufAppendF(&result, "\e[3%dm", i);
            ffStrbufAppendS(&result, symbol);
        }
        ffStrbufTrimRight(&result, ' ');
    }

    if (result.length > 0)
    {
        if (flag)
            ffLogoPrintLine();
        else
        {
            ffPrintLogoAndKey(FF_COLORS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
            flag = true;
        }

        if(options->paddingLeft > 0)
            ffPrintCharTimes(' ', options->paddingLeft);
        if(!instance.config.display.pipe)
            ffStrbufAppendS(&result, FASTFETCH_TEXT_MODIFIER_RESET);
        ffStrbufPutTo(&result, stdout);
    }

    if (!flag)
    {
        ffPrintError(FF_COLORS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", "Nothing to print");
    }
}

bool ffParseColorsCommandOptions(FFColorsOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_COLORS_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    if (ffStrEqualsIgnCase(subKey, "symbol"))
    {
        options->symbol = (FFColorsSymbol) ffOptionParseEnum(key, value, (FFKeyValuePair[]) {
            { "block", FF_COLORS_SYMBOL_BLOCK },
            { "circle", FF_COLORS_SYMBOL_CIRCLE },
            { "diamond", FF_COLORS_SYMBOL_DIAMOND },
            { "triangle", FF_COLORS_SYMBOL_TRIANGLE },
            { "square", FF_COLORS_SYMBOL_SQUARE },
            { "star", FF_COLORS_SYMBOL_STAR },
            {},
        });
        return true;
    }

    if (ffStrEqualsIgnCase(subKey, "padding-left"))
    {
        options->paddingLeft = ffOptionParseUInt32(key, value);
        return true;
    }

    if (ffStrEqualsIgnCase(subKey, "block-width"))
    {
        options->block.width = (uint8_t) ffOptionParseUInt32(key, value);
        return true;
    }

    if (ffStrEqualsIgnCase(subKey, "block-range-start"))
    {
        options->block.range[0] = min((uint8_t) ffOptionParseUInt32(key, value), 15);
        return true;
    }

    if (ffStrEqualsIgnCase(subKey, "block-range-end"))
    {
        options->block.range[1] = min((uint8_t) ffOptionParseUInt32(key, value), 15);
        return true;
    }

    return false;
}

void ffParseColorsJsonObject(FFColorsOptions* options, yyjson_val* module)
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

        if (ffStrEqualsIgnCase(key, "symbol"))
        {
            int value;
            const char* error = ffJsonConfigParseEnum(val, &value, (FFKeyValuePair[]) {
                { "block", FF_COLORS_SYMBOL_BLOCK },
                { "circle", FF_COLORS_SYMBOL_CIRCLE },
                { "diamond", FF_COLORS_SYMBOL_DIAMOND },
                { "triangle", FF_COLORS_SYMBOL_TRIANGLE },
                { "square", FF_COLORS_SYMBOL_SQUARE },
                { "star", FF_COLORS_SYMBOL_STAR },
                {},
            });
            if (error)
                ffPrintError(FF_COLORS_MODULE_NAME, 0, NULL, FF_PRINT_TYPE_NO_CUSTOM_KEY, "Invalid %s value: %s", key, error);
            else
                options->symbol = (FFColorsSymbol) value;
            continue;
        }

        if (ffStrEqualsIgnCase(key, "paddingLeft"))
        {
            options->paddingLeft = (uint32_t) yyjson_get_uint(val);
            continue;
        }

        if (ffStrEqualsIgnCase(key, "block"))
        {
            if (!yyjson_is_obj(val))
                ffPrintError(FF_COLORS_MODULE_NAME, 0, NULL, FF_PRINT_TYPE_NO_CUSTOM_KEY, "Invalid %s value: must be an object", key);
            else
            {
                yyjson_val* width = yyjson_obj_get(val, "width");
                if (width)
                    options->block.width = (uint8_t) yyjson_get_uint(width);

                yyjson_val* range = yyjson_obj_get(val, "range");
                if (range)
                {
                    if (!yyjson_is_arr(range) || yyjson_arr_size(range) != 2)
                        ffPrintError(FF_COLORS_MODULE_NAME, 0, NULL, FF_PRINT_TYPE_NO_CUSTOM_KEY, "Invalid %s.range value: must be an array of 2 elements", key);
                    else
                    {
                        uint8_t start = (uint8_t) yyjson_get_uint(yyjson_arr_get(range, 0));
                        uint8_t end = (uint8_t) yyjson_get_uint(yyjson_arr_get(range, 1));
                        if (start > end)
                            ffPrintError(FF_COLORS_MODULE_NAME, 0, NULL, FF_PRINT_TYPE_NO_CUSTOM_KEY, "Invalid %s.range value: range[0] > range[1]", key);
                        else if (end > 15)
                            ffPrintError(FF_COLORS_MODULE_NAME, 0, NULL, FF_PRINT_TYPE_NO_CUSTOM_KEY, "Invalid %s.range value: range[1] > 15", key);
                        else
                        {
                            options->block.range[0] = start;
                            options->block.range[1] = end;
                        }
                    }
                }
            }
            continue;
        }

        ffPrintError(FF_COLORS_MODULE_NAME, 0, NULL, FF_PRINT_TYPE_NO_CUSTOM_KEY, "Unknown JSON key %s", key);
    }
}

void ffGenerateColorsJsonConfig(FFColorsOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyColorsOptions))) FFColorsOptions defaultOptions;
    ffInitColorsOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);

    if (defaultOptions.symbol != options->symbol)
    {
        switch (options->symbol)
        {
            case FF_COLORS_SYMBOL_CIRCLE: yyjson_mut_obj_add_str(doc, module, "symbol", "circle"); break;
            case FF_COLORS_SYMBOL_DIAMOND: yyjson_mut_obj_add_str(doc, module, "symbol", "diamond"); break;
            case FF_COLORS_SYMBOL_TRIANGLE: yyjson_mut_obj_add_str(doc, module, "symbol", "triangle"); break;
            case FF_COLORS_SYMBOL_SQUARE: yyjson_mut_obj_add_str(doc, module, "symbol", "square"); break;
            case FF_COLORS_SYMBOL_STAR: yyjson_mut_obj_add_str(doc, module, "symbol", "star"); break;
            default: yyjson_mut_obj_add_str(doc, module, "symbol", "block"); break;
        }
    }

    if (defaultOptions.paddingLeft != options->paddingLeft)
        yyjson_mut_obj_add_uint(doc, module, "paddingLeft", options->paddingLeft);

    {
        yyjson_mut_val* block = yyjson_mut_obj(doc);

        if (defaultOptions.block.width != options->block.width)
            yyjson_mut_obj_add_uint(doc, block, "width", options->block.width);

        if (memcmp(defaultOptions.block.range, options->block.range, sizeof(options->block.range)) != 0)
        {
            yyjson_mut_val* range = yyjson_mut_obj_add_arr(doc, block, "range");
            for (uint8_t i = 0; i < 2; i++)
                yyjson_mut_arr_add_uint(doc, range, options->block.range[i]);
        }

        if (yyjson_mut_obj_size(block) > 0)
            yyjson_mut_obj_add_val(doc, module, "block", block);
    }
}

void ffInitColorsOptions(FFColorsOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_COLORS_MODULE_NAME,
        "Print some colored blocks",
        ffParseColorsCommandOptions,
        ffParseColorsJsonObject,
        ffPrintColors,
        NULL,
        NULL,
        ffGenerateColorsJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs);
    ffStrbufSetStatic(&options->moduleArgs.key, " ");
    options->symbol = FF_COLORS_SYMBOL_BLOCK;
    options->paddingLeft = 0;
    options->block = (FFBlockConfig) {
        .width = 3,
        .range = { 0, 15 },
    };
}

void ffDestroyColorsOptions(FF_MAYBE_UNUSED FFColorsOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

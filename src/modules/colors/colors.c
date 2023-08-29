#include "common/printing.h"
#include "common/jsonconfig.h"
#include "util/textModifier.h"
#include "modules/colors/colors.h"
#include "util/stringUtils.h"

void ffPrintColors(FFColorsOptions* options)
{
    if(instance.config.pipe)
        return;

    ffLogoPrintLine();

    if(options->paddingLeft > 0)
        ffPrintCharTimes(' ', options->paddingLeft);

    if (options->symbol == FF_COLORS_SYMBOL_BLOCK)
    {
        // 4%d: Set the background color
        // 3%d: Set the foreground color
        for(uint8_t i = 0; i < 8; i++)
            printf("\033[4%d;3%dm███", i, i);

        puts(FASTFETCH_TEXT_MODIFIER_RESET);

        ffLogoPrintLine();

        if(options->paddingLeft > 0)
            ffPrintCharTimes(' ', options->paddingLeft);

        // 1: Set everything to bolt. This causes normal colors on some systems to be bright.
        // 4%d: Set the backgound to the not bright color
        // 3%d: Set the foreground to the not bright color
        // 10%d: Set the background to the bright color
        // 9%d: Set the foreground to the bright color
        for(uint8_t i = 0; i < 8; i++)
            printf("\033[1;4%d;3%d;10%d;9%dm███", i, i, i, i);
    }
    else
    {
        const char* symbol;
        switch (options->symbol)
        {
            case FF_COLORS_SYMBOL_CIRCLE: symbol = "●"; break;
            case FF_COLORS_SYMBOL_DIAMOND: symbol = "◆"; break;
            case FF_COLORS_SYMBOL_TRIANGLE: symbol = "▲"; break;
            case FF_COLORS_SYMBOL_SQUARE: symbol = "■"; break;
            case FF_COLORS_SYMBOL_STAR: symbol = "★"; break;
            default: symbol = "███"; break;
        }
        for (int i = 8; i >= 1; --i)
            printf("\e[3%dm%s ", i, symbol);
    }

    puts(FASTFETCH_TEXT_MODIFIER_RESET);
}

void ffInitColorsOptions(FFColorsOptions* options)
{
    ffOptionInitModuleBaseInfo(&options->moduleInfo, FF_COLORS_MODULE_NAME, ffParseColorsCommandOptions, ffParseColorsJsonObject, ffPrintColors, NULL);
    options->symbol = FF_COLORS_SYMBOL_BLOCK;
    options->paddingLeft = 0;
}

bool ffParseColorsCommandOptions(FFColorsOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_COLORS_MODULE_NAME);
    if (!subKey) return false;

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

    return false;
}

void ffDestroyColorsOptions(FF_MAYBE_UNUSED FFColorsOptions* options)
{
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
                ffPrintErrorString(FF_COLORS_MODULE_NAME, 0, NULL, FF_PRINT_TYPE_NO_CUSTOM_KEY, "Invalid %s value: %s", key, error);
            else
                options->symbol = (FFColorsSymbol) value;
            continue;
        }

        if (ffStrEqualsIgnCase(key, "paddingLeft"))
        {
            options->paddingLeft = (uint32_t) yyjson_get_uint(val);
            continue;
        }

        ffPrintErrorString(FF_COLORS_MODULE_NAME, 0, NULL, FF_PRINT_TYPE_NO_CUSTOM_KEY, "Unknown JSON key %s", key);
    }
}

#include "common/printing.h"
#include "common/jsonconfig.h"
#include "logo/logo.h"
#include "modules/colors/colors.h"
#include "util/textModifier.h"
#include "util/stringUtils.h"

static inline uint8_t min(uint8_t a, uint8_t b)
{
    return a < b ? a : b;
}

static inline uint8_t max(uint8_t a, uint8_t b)
{
    return a > b ? a : b;
}

bool ffPrintColors(FFColorsOptions* options)
{
    bool flag = false;

    FF_STRBUF_AUTO_DESTROY result = ffStrbufCreateA(128);

    if (options->symbol == FF_COLORS_SYMBOL_BLOCK || options->symbol == FF_COLORS_SYMBOL_BACKGROUND)
    {
        // 3%d: Set the foreground color
        for(uint8_t i = options->block.range[0]; i <= min(options->block.range[1], 7); i++)
        {
            if (options->symbol == FF_COLORS_SYMBOL_BLOCK)
            {
                if (!instance.config.display.pipe)
                    ffStrbufAppendF(&result, "\e[3%dm", i);
                for (uint8_t j = 0; j < options->block.width; j++)
                    ffStrbufAppendS(&result, "█");
            }
            else
            {
                ffStrbufAppendF(&result, "\e[4%dm", i);
                ffStrbufAppendNC(&result, options->block.width, ' ');
            }
        }
        if (result.length > 0)
        {
            ffPrintLogoAndKey(FF_COLORS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
            flag = true;

            if (options->paddingLeft > 0)
                ffPrintCharTimes(' ', options->paddingLeft);

            if (!instance.config.display.pipe || options->symbol == FF_COLORS_SYMBOL_BACKGROUND)
                ffStrbufAppendS(&result, FASTFETCH_TEXT_MODIFIER_RESET);
            ffStrbufPutTo(&result, stdout);
            ffStrbufClear(&result);
        }

        #ifdef __linux__
        // Required by Linux Console for light background to work
        if (options->symbol == FF_COLORS_SYMBOL_BACKGROUND)
        {
            const char* term = getenv("TERM");
            // Should be "linux", however some terminal mulitplexer overrides $TERM
            if (term && !ffStrStartsWith(term, "xterm"))
                ffStrbufAppendS(&result, "\e[5m");
        }
        #endif

        // 9%d: Set the foreground to the bright color
        for(uint8_t i = max(options->block.range[0], 8); i <= options->block.range[1]; i++)
        {
            if (options->symbol == FF_COLORS_SYMBOL_BLOCK)
            {
                if(!instance.config.display.pipe)
                    ffStrbufAppendF(&result, "\e[9%dm", i - 8);
                for (uint8_t j = 0; j < options->block.width; j++)
                    ffStrbufAppendS(&result, "█");
            }
            else
            {
                ffStrbufAppendF(&result, "\e[10%dm", i - 8);
                ffStrbufAppendNC(&result, options->block.width, ' ');
            }
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
        if(!instance.config.display.pipe || options->symbol == FF_COLORS_SYMBOL_BACKGROUND)
            ffStrbufAppendS(&result, FASTFETCH_TEXT_MODIFIER_RESET);
        ffStrbufPutTo(&result, stdout);
    }

    if (!flag)
    {
        ffPrintError(FF_COLORS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", "Nothing to print");
        return false;
    }

    return true;
}

void ffParseColorsJsonObject(FFColorsOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        if (unsafe_yyjson_equals_str(key, "symbol"))
        {
            int value;
            const char* error = ffJsonConfigParseEnum(val, &value, (FFKeyValuePair[]) {
                { "block", FF_COLORS_SYMBOL_BLOCK },
                { "background", FF_COLORS_SYMBOL_BACKGROUND },
                { "circle", FF_COLORS_SYMBOL_CIRCLE },
                { "diamond", FF_COLORS_SYMBOL_DIAMOND },
                { "triangle", FF_COLORS_SYMBOL_TRIANGLE },
                { "square", FF_COLORS_SYMBOL_SQUARE },
                { "star", FF_COLORS_SYMBOL_STAR },
                {},
            });
            if (error)
                ffPrintError(FF_COLORS_MODULE_NAME, 0, NULL, FF_PRINT_TYPE_NO_CUSTOM_KEY, "Invalid %s value: %s", unsafe_yyjson_get_str(key), error);
            else
                options->symbol = (FFColorsSymbol) value;
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "paddingLeft"))
        {
            options->paddingLeft = (uint32_t) yyjson_get_uint(val);
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "block"))
        {
            if (!yyjson_is_obj(val))
                ffPrintError(FF_COLORS_MODULE_NAME, 0, NULL, FF_PRINT_TYPE_NO_CUSTOM_KEY, "Invalid %s value: must be an object", unsafe_yyjson_get_str(key));
            else
            {
                yyjson_val* width = yyjson_obj_get(val, "width");
                if (width)
                    options->block.width = (uint8_t) yyjson_get_uint(width);

                yyjson_val* range = yyjson_obj_get(val, "range");
                if (range)
                {
                    if (!yyjson_is_arr(range) || yyjson_arr_size(range) != 2)
                        ffPrintError(FF_COLORS_MODULE_NAME, 0, NULL, FF_PRINT_TYPE_NO_CUSTOM_KEY, "Invalid %s.range value: must be an array of 2 elements", unsafe_yyjson_get_str(key));
                    else
                    {
                        uint8_t start = (uint8_t) yyjson_get_uint(yyjson_arr_get(range, 0));
                        uint8_t end = (uint8_t) yyjson_get_uint(yyjson_arr_get(range, 1));
                        if (start > end)
                            ffPrintError(FF_COLORS_MODULE_NAME, 0, NULL, FF_PRINT_TYPE_NO_CUSTOM_KEY, "Invalid %s.range value: range[0] > range[1]", unsafe_yyjson_get_str(key));
                        else if (end > 15)
                            ffPrintError(FF_COLORS_MODULE_NAME, 0, NULL, FF_PRINT_TYPE_NO_CUSTOM_KEY, "Invalid %s.range value: range[1] > 15", unsafe_yyjson_get_str(key));
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

        ffPrintError(FF_COLORS_MODULE_NAME, 0, NULL, FF_PRINT_TYPE_NO_CUSTOM_KEY, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateColorsJsonConfig(FFColorsOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);

    yyjson_mut_obj_remove_key(module, "format"); // Not supported

    switch (options->symbol)
    {
        case FF_COLORS_SYMBOL_CIRCLE: yyjson_mut_obj_add_str(doc, module, "symbol", "circle"); break;
        case FF_COLORS_SYMBOL_DIAMOND: yyjson_mut_obj_add_str(doc, module, "symbol", "diamond"); break;
        case FF_COLORS_SYMBOL_TRIANGLE: yyjson_mut_obj_add_str(doc, module, "symbol", "triangle"); break;
        case FF_COLORS_SYMBOL_SQUARE: yyjson_mut_obj_add_str(doc, module, "symbol", "square"); break;
        case FF_COLORS_SYMBOL_STAR: yyjson_mut_obj_add_str(doc, module, "symbol", "star"); break;
        default: yyjson_mut_obj_add_str(doc, module, "symbol", "block"); break;
    }

    yyjson_mut_obj_add_uint(doc, module, "paddingLeft", options->paddingLeft);

    {
        yyjson_mut_val* block = yyjson_mut_obj_add_obj(doc, module, "block");

        yyjson_mut_obj_add_uint(doc, block, "width", options->block.width);

        yyjson_mut_val* range = yyjson_mut_obj_add_arr(doc, block, "range");
        for (uint8_t i = 0; i < 2; i++)
            yyjson_mut_arr_add_uint(doc, range, options->block.range[i]);
    }
}

void ffInitColorsOptions(FFColorsOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "");
    ffStrbufSetStatic(&options->moduleArgs.key, " ");
    options->symbol = FF_COLORS_SYMBOL_BACKGROUND;
    options->paddingLeft = 0;
    options->block = (FFBlockConfig) {
        .width = 3,
        .range = { 0, 15 },
    };
}

void ffDestroyColorsOptions(FFColorsOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffColorsModuleInfo = {
    .name = FF_COLORS_MODULE_NAME,
    .description = "Print some colored blocks",
    .initOptions = (void*) ffInitColorsOptions,
    .destroyOptions = (void*) ffDestroyColorsOptions,
    .parseJsonObject = (void*) ffParseColorsJsonObject,
    .printModule = (void*) ffPrintColors,
    .generateJsonConfig = (void*) ffGenerateColorsJsonConfig,
};

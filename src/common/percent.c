#include "fastfetch.h"
#include "common/percent.h"
#include "common/color.h"
#include "common/option.h"
#include "common/jsonconfig.h"
#include "util/textModifier.h"
#include "util/stringUtils.h"

static void appendOutputColor(FFstrbuf* buffer, const FFModuleArgs* module)
{
    if (module->outputColor.length)
        ffStrbufAppendF(buffer, "\e[%sm", module->outputColor.chars);
    else if (instance.config.display.colorOutput.length)
        ffStrbufAppendF(buffer, "\e[%sm", instance.config.display.colorOutput.chars);
}

const char* ffPercentParseTypeJsonConfig(yyjson_val* jsonVal, FFPercentageTypeFlags* result)
{
    if (yyjson_is_uint(jsonVal))
    {
        *result = (FFPercentageTypeFlags) yyjson_get_uint(jsonVal);
        return NULL;
    }
    if (yyjson_is_arr(jsonVal))
    {
        FFPercentageTypeFlags flags = 0;

        yyjson_val* item;
        size_t idx, max;
        yyjson_arr_foreach(jsonVal, idx, max, item)
        {
            const char* flag = yyjson_get_str(item);
            if (!flag)
                return "Error: percent.type: invalid flag string";
            if (ffStrEqualsIgnCase(flag, "num"))
                flags |= FF_PERCENTAGE_TYPE_NUM_BIT;
            else if (ffStrEqualsIgnCase(flag, "bar"))
                flags |= FF_PERCENTAGE_TYPE_BAR_BIT;
            else if (ffStrEqualsIgnCase(flag, "hide-others"))
                flags |= FF_PERCENTAGE_TYPE_HIDE_OTHERS_BIT;
            else if (ffStrEqualsIgnCase(flag, "num-color"))
                flags |= FF_PERCENTAGE_TYPE_NUM_COLOR_BIT;
            else if (ffStrEqualsIgnCase(flag, "bar-monochrome"))
                flags |= FF_PERCENTAGE_TYPE_BAR_MONOCHROME_BIT;
            else
                return "Error: percent.type: unknown flag string";
        }

        *result = flags;
        return NULL;
    }

    return "Error: usage: percent.type must be a number or an array of strings";
}

void ffPercentAppendBar(FFstrbuf* buffer, double percent, FFPercentageModuleConfig config, const FFModuleArgs* module)
{
    uint8_t green = config.green, yellow = config.yellow;
    assert(green <= 100 && yellow <= 100);

    const FFOptionsDisplay* options = &instance.config.display;

    uint32_t blocksPercent = (uint32_t) (percent / 100.0 * options->barWidth + 0.5);
    assert(blocksPercent <= options->barWidth);

    if(options->barBorderLeft.length)
    {
        if(!options->pipe)
            ffStrbufAppendS(buffer, "\e[" FF_COLOR_FG_LIGHT_WHITE "m");
        ffStrbufAppend(buffer, &options->barBorderLeft);
    }

    if (percent != percent)
    {
        if(!options->pipe)
            ffStrbufAppendS(buffer, "\e[" FF_COLOR_FG_LIGHT_BLACK "m");

        for (uint32_t i = 0; i < options->barWidth; ++i)
            ffStrbufAppend(buffer, &options->barCharElapsed);
    }
    else
    {
        const char* colorGreen = options->percentColorGreen.chars;
        const char* colorYellow = options->percentColorYellow.chars;
        const char* colorRed = options->percentColorRed.chars;

        FFPercentageTypeFlags percentType = config.type == 0 ? options->percentType : config.type;
        bool monochrome = !!(percentType & FF_PERCENTAGE_TYPE_BAR_MONOCHROME_BIT);

        for (uint32_t i = 0; i < blocksPercent; ++i)
        {
            if(!options->pipe)
            {
                if (monochrome)
                {
                    const char* color = NULL;
                    if (green <= yellow)
                    {
                        if (percent < green) color = colorGreen;
                        else if (percent < yellow) color = colorYellow;
                        else color = colorRed;
                    }
                    else
                    {
                        if (percent < yellow) color = colorRed;
                        else if (percent < green) color = colorYellow;
                        else color = colorGreen;
                    }
                    ffStrbufAppendF(buffer, "\e[%sm", color);
                }
                else
                {
                    uint32_t section1Begin = (uint32_t) ((green <= yellow ? green : yellow) / 100.0 * options->barWidth + 0.5);
                    uint32_t section2Begin = (uint32_t) ((green > yellow ? green : yellow) / 100.0 * options->barWidth + 0.5);
                    if (i == section2Begin)
                        ffStrbufAppendF(buffer, "\e[%sm", (green > yellow ? colorGreen : colorRed));
                    else if (i == section1Begin)
                        ffStrbufAppendF(buffer, "\e[%sm", colorYellow);
                    else if (i == 0)
                        ffStrbufAppendF(buffer, "\e[%sm", (green <= yellow ? colorGreen : colorRed));
                }
            }
            ffStrbufAppend(buffer, &options->barCharElapsed);
        }

        if (blocksPercent < options->barWidth)
        {
            if(!options->pipe)
                ffStrbufAppendS(buffer, "\e[" FF_COLOR_FG_LIGHT_WHITE "m");
            for (uint32_t i = blocksPercent; i < options->barWidth; ++i)
                ffStrbufAppend(buffer, &options->barCharTotal);
        }
    }

    if(options->barBorderRight.length)
    {
        if(!options->pipe)
            ffStrbufAppendS(buffer, "\e[" FF_COLOR_FG_LIGHT_WHITE "m");
        ffStrbufAppend(buffer, &options->barBorderRight);
    }

    if(!options->pipe)
    {
        ffStrbufAppendS(buffer, FASTFETCH_TEXT_MODIFIER_RESET);
        appendOutputColor(buffer, module);
    }
}

void ffPercentAppendNum(FFstrbuf* buffer, double percent, FFPercentageModuleConfig config, bool parentheses, const FFModuleArgs* module)
{
    uint8_t green = config.green, yellow = config.yellow;
    assert(green <= 100 && yellow <= 100);

    const FFOptionsDisplay* options = &instance.config.display;
    FFPercentageTypeFlags percentType = config.type == 0 ? options->percentType : config.type;

    bool colored = !!(percentType & FF_PERCENTAGE_TYPE_NUM_COLOR_BIT);

    if (parentheses)
        ffStrbufAppendC(buffer, '(');

    if (colored && !options->pipe)
    {
        const char* colorGreen = options->percentColorGreen.chars;
        const char* colorYellow = options->percentColorYellow.chars;
        const char* colorRed = options->percentColorRed.chars;

        if(percent != percent)
            ffStrbufAppendS(buffer, "\e[" FF_COLOR_FG_LIGHT_BLACK "m");
        else if(green <= yellow)
        {
            if (percent > yellow)
                ffStrbufAppendF(buffer, "\e[%sm", colorRed);
            else if (percent > green)
                ffStrbufAppendF(buffer, "\e[%sm", colorYellow);
            else
                ffStrbufAppendF(buffer, "\e[%sm", colorGreen);

        }
        else
        {
            if (percent < yellow)
                ffStrbufAppendF(buffer, "\e[%sm", colorRed);
            else if (percent < green)
                ffStrbufAppendF(buffer, "\e[%sm", colorYellow);
            else
                ffStrbufAppendF(buffer, "\e[%sm", colorGreen);
        }
    }
    ffStrbufAppendF(buffer, "%.*f%%", options->percentNdigits, percent);

    if (colored && !options->pipe)
    {
        ffStrbufAppendS(buffer, FASTFETCH_TEXT_MODIFIER_RESET);
        appendOutputColor(buffer, module);
    }

    if (parentheses)
        ffStrbufAppendC(buffer, ')');
}

bool ffPercentParseCommandOptions(const char* key, const char* subkey, const char* value, FFPercentageModuleConfig* config)
{
    if (!ffStrStartsWithIgnCase(subkey, "percent-"))
        return false;

    subkey += strlen("percent-");

    if (ffStrEqualsIgnCase(subkey, "green"))
    {
        uint32_t num = ffOptionParseUInt32(key, value);
        if (num > 100)
        {
            fprintf(stderr, "Error: usage: %s must be between 0 and 100\n", key);
            exit(480);
        }
        config->green = (uint8_t) num;
        return true;
    }

    if (ffStrEqualsIgnCase(subkey, "yellow"))
    {
        uint32_t num = ffOptionParseUInt32(key, value);
        if (num > 100)
        {
            fprintf(stderr, "Error: usage: %s must be between 0 and 100\n", key);
            exit(480);
        }
        config->yellow = (uint8_t) num;
        return true;
    }

    if (ffStrEqualsIgnCase(subkey, "type"))
    {
        config->type = (FFPercentageTypeFlags) ffOptionParseUInt32(key, value);
        return true;
    }

    return false;
}

bool ffPercentParseJsonObject(const char* key, yyjson_val* value, FFPercentageModuleConfig* config)
{
    if (!ffStrEqualsIgnCase(key, "percent"))
        return false;

    if (!yyjson_is_obj(value))
    {
        fprintf(stderr, "Error: usage: %s must be an object\n", key);
        exit(480);
    }

    yyjson_val* greenVal = yyjson_obj_get(value, "green");
    if (greenVal)
    {
        int num = yyjson_get_int(greenVal);
        if (num < 0 || num > 100)
        {
            fputs("Error: usage: percent.green must be between 0 and 100\n", stderr);
            exit(480);
        }
        config->green = (uint8_t) num;
    }

    yyjson_val* yellowVal = yyjson_obj_get(value, "yellow");
    if (yellowVal)
    {
        int num = yyjson_get_int(yellowVal);
        if (num < 0 || num > 100)
        {
            fputs("Error: usage: percent.yellow must be between 0 and 100\n", stderr);
            exit(480);
        }
        config->yellow = (uint8_t) num;
    }

    yyjson_val* typeVal = yyjson_obj_get(value, "type");
    if (typeVal)
    {
        const char* error = ffPercentParseTypeJsonConfig(typeVal, &config->type);
        if (error)
        {
            fputs(error, stderr);
            exit(480);
        }
    }

    return true;
}

void ffPercentGenerateJsonConfig(yyjson_mut_doc* doc, yyjson_mut_val* module, FFPercentageModuleConfig defaultConfig, FFPercentageModuleConfig config)
{
    if (config.green == defaultConfig.green && config.yellow == defaultConfig.yellow)
        return;

    yyjson_mut_val* percent = yyjson_mut_obj_add_obj(doc, module, "percent");
    if (config.green != defaultConfig.green)
        yyjson_mut_obj_add_uint(doc, percent, "green", config.green);
    if (config.yellow != defaultConfig.yellow)
        yyjson_mut_obj_add_uint(doc, percent, "yellow", config.yellow);
    if (config.type != defaultConfig.type)
        yyjson_mut_obj_add_uint(doc, percent, "type", config.type);
}

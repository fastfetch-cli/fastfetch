#include "fastfetch.h"
#include "common/percent.h"
#include "common/color.h"
#include "common/option.h"
#include "common/jsonconfig.h"
#include "util/textModifier.h"
#include "util/stringUtils.h"

void ffPercentAppendBar(FFstrbuf* buffer, double percent, FFPercentConfig config)
{
    uint8_t green = config.green, yellow = config.yellow;
    assert(green <= 100 && yellow <= 100);

    const FFOptionsDisplay* options = &instance.config.display;

    uint32_t blocksPercent = (uint32_t) (percent / 100.0 * options->barWidth + 0.5);
    assert(blocksPercent <= options->barWidth);

    if(options->barBorder)
    {
        if(!options->pipe)
            ffStrbufAppendS(buffer, "\e[" FF_COLOR_FG_LIGHT_WHITE "m[ ");
        else
            ffStrbufAppendS(buffer, "[ ");
    }

    for (uint32_t i = 0; i < blocksPercent; ++i)
    {
        if(!options->pipe)
        {
            uint32_t section1Begin = (uint32_t) ((green <= yellow ? green : yellow) / 100.0 * options->barWidth + 0.5);
            uint32_t section2Begin = (uint32_t) ((green > yellow ? green : yellow) / 100.0 * options->barWidth + 0.5);
            if (i == section2Begin)
                ffStrbufAppendF(buffer, "\e[%sm", (green > yellow ? FF_COLOR_FG_LIGHT_GREEN : FF_COLOR_FG_LIGHT_RED));
            else if (i == section1Begin)
                ffStrbufAppendS(buffer, "\e[" FF_COLOR_FG_LIGHT_YELLOW "m");
            else if (i == 0)
                ffStrbufAppendF(buffer, "\e[%sm", (green <= yellow ? FF_COLOR_FG_GREEN : FF_COLOR_FG_LIGHT_RED));
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

    if(options->barBorder)
    {
        if(!options->pipe)
            ffStrbufAppendS(buffer, "\e[" FF_COLOR_FG_LIGHT_WHITE "m ]");
        else
            ffStrbufAppendS(buffer, " ]");
    }

    if(!options->pipe)
        ffStrbufAppendS(buffer, FASTFETCH_TEXT_MODIFIER_RESET);
}

void ffPercentAppendNum(FFstrbuf* buffer, double percent, FFPercentConfig config, bool parentheses)
{
    uint8_t green = config.green, yellow = config.yellow;
    assert(green <= 100 && yellow <= 100);

    const FFOptionsDisplay* options = &instance.config.display;

    bool colored = !!(options->percentType & FF_PERCENTAGE_TYPE_NUM_COLOR_BIT);

    if (parentheses)
        ffStrbufAppendC(buffer, '(');

    if (colored && !options->pipe)
    {
        if(percent != percent)
            ffStrbufAppendS(buffer, "\e[" FF_COLOR_FG_LIGHT_BLACK "m");
        else if(green <= yellow)
        {
            if (percent > yellow)
                ffStrbufAppendS(buffer, "\e[" FF_COLOR_FG_LIGHT_RED "m");
            else if (percent > green)
                ffStrbufAppendS(buffer, "\e[" FF_COLOR_FG_LIGHT_YELLOW "m");
            else
                ffStrbufAppendS(buffer, "\e[" FF_COLOR_FG_GREEN "m");

        }
        else
        {
            if (percent < yellow)
                ffStrbufAppendS(buffer, "\e[" FF_COLOR_FG_LIGHT_RED "m");
            else if (percent < green)
                ffStrbufAppendS(buffer, "\e[" FF_COLOR_FG_LIGHT_YELLOW "m");
            else
                ffStrbufAppendS(buffer, "\e[" FF_COLOR_FG_GREEN "m");
        }
    }
    ffStrbufAppendF(buffer, "%.*f%%", options->percentNdigits, percent);

    if (colored && !options->pipe)
    {
        ffStrbufAppendS(buffer, FASTFETCH_TEXT_MODIFIER_RESET);
    }

    if (parentheses)
        ffStrbufAppendC(buffer, ')');
}

bool ffPercentParseCommandOptions(const char* key, const char* subkey, const char* value, FFPercentConfig* config)
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

    return false;
}

bool ffPercentParseJsonObject(const char* key, yyjson_val* value, FFPercentConfig* config)
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

    return true;
}

void ffPercentGenerateJsonConfig(yyjson_mut_doc* doc, yyjson_mut_val* module, FFPercentConfig defaultConfig, FFPercentConfig config)
{
    if (config.green == defaultConfig.green && config.yellow == defaultConfig.yellow)
        return;

    yyjson_mut_val* percent = yyjson_mut_obj_add_obj(doc, module, "percent");
    if (config.green != defaultConfig.green)
        yyjson_mut_obj_add_uint(doc, percent, "green", config.green);
    if (config.yellow != defaultConfig.yellow)
        yyjson_mut_obj_add_uint(doc, percent, "yellow", config.yellow);
}

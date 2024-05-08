#include "fastfetch.h"
#include "common/temps.h"
#include "util/textModifier.h"
#include "util/stringUtils.h"

void ffTempsAppendNum(double celsius, FFstrbuf* buffer, FFColorRangeConfig config, const FFModuleArgs* module)
{
    if (celsius != celsius) // ignores NaN
        return;

    const FFOptionsDisplay* options = &instance.config.display;
    const char* colorGreen = options->tempColorGreen.chars;
    const char* colorYellow = options->tempColorYellow.chars;
    const char* colorRed = options->tempColorRed.chars;

    uint8_t green = config.green, yellow = config.yellow;

    if (!options->pipe)
    {
        if(green <= yellow)
        {
            if (celsius > yellow)
                ffStrbufAppendF(buffer, "\e[%sm", colorRed);
            else if (celsius > green)
                ffStrbufAppendF(buffer, "\e[%sm", colorYellow);
            else
                ffStrbufAppendF(buffer, "\e[%sm", colorGreen);
        }
        else
        {
            if (celsius < yellow)
                ffStrbufAppendF(buffer, "\e[%sm", colorRed);
            else if (celsius < green)
                ffStrbufAppendF(buffer, "\e[%sm", colorYellow);
            else
                ffStrbufAppendF(buffer, "\e[%sm", colorGreen);
        }
    }

    switch (options->temperatureUnit)
    {
        case FF_TEMPERATURE_UNIT_CELSIUS:
            ffStrbufAppendF(buffer, "%.*f°C", options->tempNdigits, celsius);
            break;
        case FF_TEMPERATURE_UNIT_FAHRENHEIT:
            ffStrbufAppendF(buffer, "%.*f°F", options->tempNdigits, celsius * 1.8 + 32);
            break;
        case FF_TEMPERATURE_UNIT_KELVIN:
            ffStrbufAppendF(buffer, "%.*f K", options->tempNdigits, celsius + 273.15);
            break;
    }

    if (!options->pipe)
    {
        ffStrbufAppendS(buffer, FASTFETCH_TEXT_MODIFIER_RESET);
        if (module->outputColor.length)
            ffStrbufAppendF(buffer, "\e[%sm", module->outputColor.chars);
        else if (instance.config.display.colorOutput.length)
            ffStrbufAppendF(buffer, "\e[%sm", instance.config.display.colorOutput.chars);
    }
}

bool ffTempsParseCommandOptions(const char* key, const char* subkey, const char* value, bool* useTemp, FFColorRangeConfig* config)
{
    if (!ffStrStartsWithIgnCase(subkey, "temp"))
        return false;

    if (subkey[strlen("temp")] == '\0')
    {
        *useTemp = ffOptionParseBoolean(value);
        return true;
    }

    if (subkey[strlen("temp")] != '-')
        return false;

    subkey += strlen("temp-");

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

bool ffTempsParseJsonObject(const char* key, yyjson_val* value, bool* useTemp, FFColorRangeConfig* config)
{
    if (!ffStrEqualsIgnCase(key, "temp"))
        return false;

    if (yyjson_is_bool(value))
    {
        *useTemp = yyjson_get_bool(value);
        return true;
    }

    if (yyjson_is_null(value))
    {
        *useTemp = false;
        return true;
    }

    if (!yyjson_is_obj(value))
    {
        fprintf(stderr, "Error: usage: %s must be an object or a boolean\n", key);
        exit(480);
    }

    *useTemp = true;

    yyjson_val* greenVal = yyjson_obj_get(value, "green");
    if (greenVal)
    {
        int num = yyjson_get_int(greenVal);
        if (num < 0 || num > 100)
        {
            fputs("Error: usage: temp.green must be between 0 and 100\n", stderr);
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
            fputs("Error: usage: temp.yellow must be between 0 and 100\n", stderr);
            exit(480);
        }
        config->yellow = (uint8_t) num;
    }

    return true;
}

void ffTempsGenerateJsonConfig(yyjson_mut_doc* doc, yyjson_mut_val* module, FF_MAYBE_UNUSED bool defaultTemp, FFColorRangeConfig defaultConfig, bool temp, FFColorRangeConfig config)
{
    assert(defaultTemp == false); // assume defaultTemp is always false

    if (!temp)
        return;

    if (config.green != defaultConfig.green || config.yellow != defaultConfig.yellow)
    {
        yyjson_mut_val* temp = yyjson_mut_obj_add_obj(doc, module, "temp");
        if (config.green != defaultConfig.green)
            yyjson_mut_obj_add_uint(doc, temp, "green", config.green);
        if (config.yellow != defaultConfig.yellow)
            yyjson_mut_obj_add_uint(doc, temp, "yellow", config.yellow);
    }
    else
    {
        yyjson_mut_obj_add_bool(doc, module, "temp", true);
    }
}

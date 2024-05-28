#include "fastfetch.h"
#include "common/color.h"
#include "common/jsonconfig.h"
#include "util/stringUtils.h"
#include "options/display.h"

#include <unistd.h>

const char* ffOptionsParseDisplayJsonConfig(FFOptionsDisplay* options, yyjson_val* root)
{
    yyjson_val* object = yyjson_obj_get(root, "display");
    if (!object) return NULL;
    if (!yyjson_is_obj(object)) return "Property 'display' must be an object";

    yyjson_val *key_, *val;
    size_t idx, max;
    yyjson_obj_foreach(object, idx, max, key_, val)
    {
        const char* key = yyjson_get_str(key_);

        if (ffStrEqualsIgnCase(key, "stat"))
        {
            if ((options->stat = yyjson_get_bool(val)))
                options->showErrors = true;
        }
        else if (ffStrEqualsIgnCase(key, "pipe"))
            options->pipe = yyjson_get_bool(val);
        else if (ffStrEqualsIgnCase(key, "showErrors"))
            options->showErrors = yyjson_get_bool(val);
        else if (ffStrEqualsIgnCase(key, "disableLinewrap"))
            options->disableLinewrap = yyjson_get_bool(val);
        else if (ffStrEqualsIgnCase(key, "hideCursor"))
            options->hideCursor = yyjson_get_bool(val);
        else if (ffStrEqualsIgnCase(key, "separator"))
            ffStrbufSetS(&options->keyValueSeparator, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "color"))
        {
            if (yyjson_is_str(val))
            {
                ffOptionParseColor(yyjson_get_str(val), &options->colorKeys);
                ffStrbufSet(&options->colorTitle, &options->colorKeys);
            }
            else if (yyjson_is_obj(val))
            {
                const char* colorKeys = yyjson_get_str(yyjson_obj_get(val, "keys"));
                if (colorKeys)
                    ffOptionParseColor(colorKeys, &options->colorKeys);
                const char* colorTitle = yyjson_get_str(yyjson_obj_get(val, "title"));
                if (colorTitle)
                    ffOptionParseColor(colorTitle, &options->colorTitle);
                const char* colorOutput = yyjson_get_str(yyjson_obj_get(val, "output"));
                if (colorOutput)
                    ffOptionParseColor(colorOutput, &options->colorOutput);
                const char* colorSeparator = yyjson_get_str(yyjson_obj_get(val, "separator"));
                if (colorSeparator)
                    ffOptionParseColor(colorSeparator, &options->colorSeparator);
            }
            else
                return "display.color must be either a string or an object";
        }
        else if (ffStrEqualsIgnCase(key, "brightColor"))
            options->brightColor = yyjson_get_bool(val);
        else if (ffStrEqualsIgnCase(key, "binaryPrefix"))
        {
            int value;
            const char* error = ffJsonConfigParseEnum(val, &value, (FFKeyValuePair[]) {
                { "iec", FF_BINARY_PREFIX_TYPE_IEC },
                { "si", FF_BINARY_PREFIX_TYPE_SI },
                { "jedec", FF_BINARY_PREFIX_TYPE_JEDEC },
                {},
            });
            if (error) return error;
            options->binaryPrefixType = (FFBinaryPrefixType) value;
        }
        else if (ffStrEqualsIgnCase(key, "size"))
        {
            if (!yyjson_is_obj(val))
                return "display.size must be an object";

            yyjson_val* maxPrefix = yyjson_obj_get(val, "maxPrefix");
            if (maxPrefix)
            {
                int value;
                const char* error = ffJsonConfigParseEnum(maxPrefix, &value, (FFKeyValuePair[]) {
                    { "B", 0 },
                    { "kB", 1 },
                    { "MB", 2 },
                    { "GB", 3 },
                    { "TB", 4 },
                    { "PB", 5 },
                    { "EB", 6 },
                    { "ZB", 7 },
                    { "YB", 8 },
                    {}
                });
                if (error) return error;
                options->sizeMaxPrefix = (uint8_t) value;
            }

            yyjson_val* ndigits = yyjson_obj_get(val, "ndigits");
            if (ndigits) options->sizeNdigits = (uint8_t) yyjson_get_uint(ndigits);
        }
        else if (ffStrEqualsIgnCase(key, "temp"))
        {
            if (!yyjson_is_obj(val))
                return "display.temp must be an object";

            yyjson_val* unit = yyjson_obj_get(val, "unit");
            if (unit)
            {
                int value;
                const char* error = ffJsonConfigParseEnum(unit, &value, (FFKeyValuePair[]) {
                    { "CELSIUS", FF_TEMPERATURE_UNIT_CELSIUS },
                    { "C", FF_TEMPERATURE_UNIT_CELSIUS },
                    { "FAHRENHEIT", FF_TEMPERATURE_UNIT_FAHRENHEIT },
                    { "F", FF_TEMPERATURE_UNIT_FAHRENHEIT },
                    { "KELVIN", FF_TEMPERATURE_UNIT_KELVIN },
                    { "K", FF_TEMPERATURE_UNIT_KELVIN },
                    {},
                });
                if (error) return error;
                options->tempUnit = (FFTemperatureUnit) value;
            }

            yyjson_val* ndigits = yyjson_obj_get(val, "ndigits");
            if (ndigits) options->tempNdigits = (uint8_t) yyjson_get_uint(ndigits);

            yyjson_val* color = yyjson_obj_get(val, "color");
            if (color)
            {
                if (!yyjson_is_obj(color))
                    return "display.temperature.color must be an object";

                yyjson_val* green = yyjson_obj_get(color, "green");
                if (green) ffOptionParseColor(yyjson_get_str(green), &options->tempColorGreen);

                yyjson_val* yellow = yyjson_obj_get(color, "yellow");
                if (yellow) ffOptionParseColor(yyjson_get_str(yellow), &options->tempColorYellow);

                yyjson_val* red = yyjson_obj_get(color, "red");
                if (red) ffOptionParseColor(yyjson_get_str(red), &options->tempColorRed);
            }
        }
        else if (ffStrEqualsIgnCase(key, "percent"))
        {
            if (!yyjson_is_obj(val))
                return "display.percent must be an object";

            yyjson_val* type = yyjson_obj_get(val, "type");
            if (type) options->percentType = (uint8_t) yyjson_get_uint(type);

            yyjson_val* ndigits = yyjson_obj_get(val, "ndigits");
            if (ndigits) options->percentNdigits = (uint8_t) yyjson_get_uint(ndigits);

            yyjson_val* color = yyjson_obj_get(val, "color");
            if (color)
            {
                if (!yyjson_is_obj(color))
                    return "display.percent.color must be an object";

                yyjson_val* green = yyjson_obj_get(color, "green");
                if (green) ffOptionParseColor(yyjson_get_str(green), &options->percentColorGreen);

                yyjson_val* yellow = yyjson_obj_get(color, "yellow");
                if (yellow) ffOptionParseColor(yyjson_get_str(yellow), &options->percentColorYellow);

                yyjson_val* red = yyjson_obj_get(color, "red");
                if (red) ffOptionParseColor(yyjson_get_str(red), &options->percentColorRed);
            }
        }
        else if (ffStrEqualsIgnCase(key, "bar"))
        {
            if (yyjson_is_obj(val))
            {
                const char* charElapsed = yyjson_get_str(yyjson_obj_get(val, "charElapsed"));
                if (charElapsed)
                    ffStrbufSetS(&options->barCharElapsed, charElapsed);

                const char* charTotal = yyjson_get_str(yyjson_obj_get(val, "charTotal"));
                if (charTotal)
                    ffStrbufSetS(&options->barCharTotal, charTotal);

                yyjson_val* border = yyjson_obj_get(val, "border");
                if (border)
                    options->barBorder = yyjson_get_bool(border);

                yyjson_val* width = yyjson_obj_get(val, "width");
                if (width)
                    options->barWidth = (uint8_t) yyjson_get_uint(width);
            }
            else
                return "display.bar must be an object";
        }
        else if (ffStrEqualsIgnCase(key, "noBuffer"))
            options->noBuffer = yyjson_get_bool(val);
        else if (ffStrEqualsIgnCase(key, "keyWidth"))
            options->keyWidth = (uint32_t) yyjson_get_uint(val);
        else if (ffStrEqualsIgnCase(key, "tsVersion"))
            options->tsVersion = yyjson_get_bool(val);
        else
            return "Unknown display property";
    }

    return NULL;
}

static inline void optionCheckString(const char* key, const char* value, FFstrbuf* buffer)
{
    if(value == NULL)
    {
        fprintf(stderr, "Error: usage: %s <str>\n", key);
        exit(477);
    }
    ffStrbufEnsureFree(buffer, 63); //This is not needed, as ffStrbufSetS will resize capacity if needed, but giving a higher start should improve performance
}

bool ffOptionsParseDisplayCommandLine(FFOptionsDisplay* options, const char* key, const char* value)
{
    if(ffStrEqualsIgnCase(key, "--stat"))
    {
        if((options->stat = ffOptionParseBoolean(value)))
            options->showErrors = true;
    }
    else if(ffStrEqualsIgnCase(key, "--pipe"))
        options->pipe = ffOptionParseBoolean(value);
    else if(ffStrEqualsIgnCase(key, "--show-errors"))
        options->showErrors = ffOptionParseBoolean(value);
    else if(ffStrEqualsIgnCase(key, "--disable-linewrap"))
        options->disableLinewrap = ffOptionParseBoolean(value);
    else if(ffStrEqualsIgnCase(key, "--hide-cursor"))
        options->hideCursor = ffOptionParseBoolean(value);
    else if(ffStrEqualsIgnCase(key, "--separator"))
        ffOptionParseString(key, value, &options->keyValueSeparator);
    else if(ffStrEqualsIgnCase(key, "--color"))
    {
        optionCheckString(key, value, &options->colorKeys);
        ffOptionParseColor(value, &options->colorKeys);
        ffStrbufSet(&options->colorTitle, &options->colorKeys);
    }
    else if(ffStrStartsWithIgnCase(key, "--color-"))
    {
        const char* subkey = key + strlen("--color-");
        if(ffStrEqualsIgnCase(subkey, "keys"))
        {
            optionCheckString(key, value, &options->colorKeys);
            ffOptionParseColor(value, &options->colorKeys);
        }
        else if(ffStrEqualsIgnCase(subkey, "title"))
        {
            optionCheckString(key, value, &options->colorTitle);
            ffOptionParseColor(value, &options->colorTitle);
        }
        else if(ffStrEqualsIgnCase(subkey, "output"))
        {
            optionCheckString(key, value, &options->colorOutput);
            ffOptionParseColor(value, &options->colorOutput);
        }
        else if(ffStrEqualsIgnCase(subkey, "separator"))
        {
            optionCheckString(key, value, &options->colorSeparator);
            ffOptionParseColor(value, &options->colorSeparator);
        }
        else
            return false;
    }
    else if(ffStrEqualsIgnCase(key, "--key-width"))
        options->keyWidth = ffOptionParseUInt32(key, value);
    else if(ffStrEqualsIgnCase(key, "--bright-color"))
        options->brightColor = ffOptionParseBoolean(value);
    else if(ffStrEqualsIgnCase(key, "--binary-prefix"))
    {
        options->binaryPrefixType = (FFBinaryPrefixType) ffOptionParseEnum(key, value, (FFKeyValuePair[]) {
            { "iec", FF_BINARY_PREFIX_TYPE_IEC },
            { "si", FF_BINARY_PREFIX_TYPE_SI },
            { "jedec", FF_BINARY_PREFIX_TYPE_JEDEC },
            {}
        });
    }
    else if(ffStrEqualsIgnCase(key, "--size-ndigits"))
        options->sizeNdigits = (uint8_t) ffOptionParseUInt32(key, value);
    else if(ffStrEqualsIgnCase(key, "--size-max-prefix"))
    {
        options->sizeMaxPrefix = (uint8_t) ffOptionParseEnum(key, value, (FFKeyValuePair[]) {
            { "B", 0 },
            { "kB", 1 },
            { "MB", 2 },
            { "GB", 3 },
            { "TB", 4 },
            { "PB", 5 },
            { "EB", 6 },
            { "ZB", 7 },
            { "YB", 8 },
            {}
        });
    }
    else if(ffStrStartsWithIgnCase(key, "--temp-"))
    {
        const char* subkey = key + strlen("--temp-");
        if(ffStrEqualsIgnCase(subkey, "unit"))
        {
            options->tempUnit = (FFTemperatureUnit) ffOptionParseEnum(key, value, (FFKeyValuePair[]) {
                { "CELSIUS", FF_TEMPERATURE_UNIT_CELSIUS },
                { "C", FF_TEMPERATURE_UNIT_CELSIUS },
                { "FAHRENHEIT", FF_TEMPERATURE_UNIT_FAHRENHEIT },
                { "F", FF_TEMPERATURE_UNIT_FAHRENHEIT },
                { "KELVIN", FF_TEMPERATURE_UNIT_KELVIN },
                { "K", FF_TEMPERATURE_UNIT_KELVIN },
                {},
            });
        }
        else if (ffStrEqualsIgnCase(subkey, "ndigits"))
            options->tempNdigits = (uint8_t) ffOptionParseUInt32(key, value);
        else if(ffStrEqualsIgnCase(subkey, "color-green"))
            ffOptionParseColor(value, &options->tempColorGreen);
        else if(ffStrEqualsIgnCase(subkey, "color-yellow"))
            ffOptionParseColor(value, &options->tempColorYellow);
        else if(ffStrEqualsIgnCase(subkey, "color-red"))
            ffOptionParseColor(value, &options->tempColorRed);
        else
            return false;
    }
    else if(ffStrStartsWithIgnCase(key, "--percent-"))
    {
        const char* subkey = key + strlen("--percent-");
        if(ffStrEqualsIgnCase(subkey, "type"))
            options->percentType = (uint8_t) ffOptionParseUInt32(key, value);
        else if(ffStrEqualsIgnCase(subkey, "ndigits"))
            options->percentNdigits = (uint8_t) ffOptionParseUInt32(key, value);
        else if(ffStrEqualsIgnCase(subkey, "color-green"))
            ffOptionParseColor(value, &options->percentColorGreen);
        else if(ffStrEqualsIgnCase(subkey, "color-yellow"))
            ffOptionParseColor(value, &options->percentColorYellow);
        else if(ffStrEqualsIgnCase(subkey, "color-red"))
            ffOptionParseColor(value, &options->percentColorRed);
        else
            return false;
    }
    else if(ffStrEqualsIgnCase(key, "--no-buffer"))
        options->noBuffer = ffOptionParseBoolean(value);
    else if(ffStrStartsWithIgnCase(key, "--bar-"))
    {
        const char* subkey = key + strlen("--bar-");
        if(ffStrEqualsIgnCase(subkey, "char-elapsed"))
            ffOptionParseString(key, value, &options->barCharElapsed);
        else if(ffStrEqualsIgnCase(subkey, "char-total"))
            ffOptionParseString(key, value, &options->barCharTotal);
        else if(ffStrEqualsIgnCase(subkey, "width"))
            options->barWidth = (uint8_t) ffOptionParseUInt32(key, value);
        else if(ffStrEqualsIgnCase(subkey, "border"))
            options->barBorder = ffOptionParseBoolean(value);
        else
            return false;
    }
    else if(ffStrEqualsIgnCase(key, "--ts-version"))
        options->tsVersion = ffOptionParseBoolean(value);
    else
        return false;
    return true;
}

void ffOptionsInitDisplay(FFOptionsDisplay* options)
{
    ffStrbufInit(&options->colorKeys);
    ffStrbufInit(&options->colorTitle);
    ffStrbufInit(&options->colorOutput);
    ffStrbufInit(&options->colorSeparator);
    options->brightColor = true;
    ffStrbufInitStatic(&options->keyValueSeparator, ": ");

    options->showErrors = false;
    options->pipe = !isatty(STDOUT_FILENO) || !!getenv("NO_COLOR");

    #ifdef NDEBUG
    options->disableLinewrap = !options->pipe;
    #else
    options->disableLinewrap = false;
    #endif

    options->hideCursor = false;
    options->binaryPrefixType = FF_BINARY_PREFIX_TYPE_IEC;
    options->sizeNdigits = 2;
    options->sizeMaxPrefix = UINT8_MAX;
    options->stat = false;
    options->noBuffer = false;
    options->keyWidth = 0;

    options->tempUnit = FF_TEMPERATURE_UNIT_CELSIUS;
    options->tempNdigits = 1;
    ffStrbufInitStatic(&options->tempColorGreen, FF_COLOR_FG_GREEN);
    ffStrbufInitStatic(&options->tempColorYellow, FF_COLOR_FG_LIGHT_YELLOW);
    ffStrbufInitStatic(&options->tempColorRed, FF_COLOR_FG_LIGHT_RED);

    ffStrbufInitStatic(&options->barCharElapsed, "■");
    ffStrbufInitStatic(&options->barCharTotal, "-");
    options->barWidth = 10;
    options->barBorder = true;
    options->percentType = 9;
    options->percentNdigits = 0;
    ffStrbufInitStatic(&options->percentColorGreen, FF_COLOR_FG_GREEN);
    ffStrbufInitStatic(&options->percentColorYellow, FF_COLOR_FG_LIGHT_YELLOW);
    ffStrbufInitStatic(&options->percentColorRed, FF_COLOR_FG_LIGHT_RED);

    options->tsVersion = true;
}

void ffOptionsDestroyDisplay(FFOptionsDisplay* options)
{
    ffStrbufDestroy(&options->colorKeys);
    ffStrbufDestroy(&options->colorTitle);
    ffStrbufDestroy(&options->colorOutput);
    ffStrbufDestroy(&options->colorSeparator);
    ffStrbufDestroy(&options->keyValueSeparator);
    ffStrbufDestroy(&options->barCharElapsed);
    ffStrbufDestroy(&options->barCharTotal);
}

void ffOptionsGenerateDisplayJsonConfig(FFOptionsDisplay* options, yyjson_mut_doc* doc)
{
    __attribute__((__cleanup__(ffOptionsDestroyDisplay))) FFOptionsDisplay defaultOptions;
    ffOptionsInitDisplay(&defaultOptions);

    yyjson_mut_val* obj = yyjson_mut_obj(doc);

    if (options->stat != defaultOptions.stat)
        yyjson_mut_obj_add_bool(doc, obj, "stat", options->stat);

    if (options->pipe != defaultOptions.pipe)
        yyjson_mut_obj_add_bool(doc, obj, "pipe", options->pipe);

    if (options->showErrors != defaultOptions.showErrors)
        yyjson_mut_obj_add_bool(doc, obj, "showErrors", options->showErrors);

    if (options->disableLinewrap != defaultOptions.disableLinewrap)
        yyjson_mut_obj_add_bool(doc, obj, "disableLinewrap", options->disableLinewrap);

    if (options->hideCursor != defaultOptions.hideCursor)
        yyjson_mut_obj_add_bool(doc, obj, "hideCursor", options->hideCursor);

    if (!ffStrbufEqual(&options->keyValueSeparator, &defaultOptions.keyValueSeparator))
        yyjson_mut_obj_add_strbuf(doc, obj, "separator", &options->keyValueSeparator);

    {
        yyjson_mut_val* color = yyjson_mut_obj(doc);
        if (!ffStrbufEqual(&options->colorKeys, &defaultOptions.colorKeys))
            yyjson_mut_obj_add_strbuf(doc, color, "keys", &options->colorKeys);
        if (!ffStrbufEqual(&options->colorTitle, &defaultOptions.colorTitle))
            yyjson_mut_obj_add_strbuf(doc, color, "title", &options->colorTitle);
        if (!ffStrbufEqual(&options->colorOutput, &defaultOptions.colorOutput))
            yyjson_mut_obj_add_strbuf(doc, color, "output", &options->colorOutput);
        if (!ffStrbufEqual(&options->colorSeparator, &defaultOptions.colorSeparator))
            yyjson_mut_obj_add_strbuf(doc, color, "separator", &options->colorSeparator);
        if (yyjson_mut_obj_size(color) > 0)
        {
            if (yyjson_mut_obj_size(color) == 2 && ffStrbufEqual(&options->colorKeys, &options->colorTitle))
                yyjson_mut_obj_add_strbuf(doc, obj, "color", &options->colorKeys);
            else
                yyjson_mut_obj_add_val(doc, obj, "color", color);
        }
    }

    if (options->brightColor != defaultOptions.brightColor)
        yyjson_mut_obj_add_bool(doc, obj, "brightColor", options->brightColor);

    if (options->binaryPrefixType != defaultOptions.binaryPrefixType)
    {
        switch (options->binaryPrefixType)
        {
            case FF_BINARY_PREFIX_TYPE_IEC:
                yyjson_mut_obj_add_str(doc, obj, "binaryPrefix", "iec");
                break;
            case FF_BINARY_PREFIX_TYPE_SI:
                yyjson_mut_obj_add_str(doc, obj, "binaryPrefix", "si");
                break;
            case FF_BINARY_PREFIX_TYPE_JEDEC:
                yyjson_mut_obj_add_str(doc, obj, "binaryPrefix", "jedec");
                break;

        }
    }

    {
        yyjson_mut_val* size = yyjson_mut_obj(doc);
        if (options->sizeNdigits != defaultOptions.sizeNdigits)
            yyjson_mut_obj_add_uint(doc, size, "ndigits", options->sizeNdigits);
        if (options->sizeMaxPrefix != defaultOptions.sizeMaxPrefix && options->sizeMaxPrefix <= 8)
        {
            yyjson_mut_obj_add_str(doc, size, "maxPrefix", ((const char* []) {
                "B",
                "kB",
                "MB",
                "GB",
                "TB",
                "PB",
                "EB",
                "ZB",
                "YB",
            })[options->sizeMaxPrefix]);
        }
        if (yyjson_mut_obj_size(size) > 0)
            yyjson_mut_obj_add_val(doc, obj, "size", size);
    }

    {
        yyjson_mut_val* temperature = yyjson_mut_obj(doc);
        if (options->tempUnit != defaultOptions.tempUnit)
        {
            switch (options->tempUnit)
            {
                case FF_TEMPERATURE_UNIT_CELSIUS:
                    yyjson_mut_obj_add_str(doc, obj, "unit", "C");
                    break;
                case FF_TEMPERATURE_UNIT_FAHRENHEIT:
                    yyjson_mut_obj_add_str(doc, obj, "unit", "F");
                    break;
                case FF_TEMPERATURE_UNIT_KELVIN:
                    yyjson_mut_obj_add_str(doc, obj, "unit", "K");
                    break;
            }
        }
        if (options->tempNdigits != defaultOptions.tempNdigits)
            yyjson_mut_obj_add_uint(doc, temperature, "ndigits", options->tempNdigits);
        {
            yyjson_mut_val* color = yyjson_mut_obj(doc);
            if (!ffStrbufEqual(&options->tempColorGreen, &defaultOptions.tempColorGreen))
                yyjson_mut_obj_add_strbuf(doc, color, "green", &options->tempColorGreen);
            if (!ffStrbufEqual(&options->tempColorYellow, &defaultOptions.tempColorYellow))
                yyjson_mut_obj_add_strbuf(doc, color, "yellow", &options->tempColorYellow);
            if (!ffStrbufEqual(&options->tempColorRed, &defaultOptions.tempColorRed))
                yyjson_mut_obj_add_strbuf(doc, color, "red", &options->tempColorRed);
            if (yyjson_mut_obj_size(color) > 0)
                yyjson_mut_obj_add_val(doc, temperature, "color", color);
        }
        if (yyjson_mut_obj_size(temperature) > 0)
            yyjson_mut_obj_add_val(doc, obj, "temp", temperature);
    }

    {
        yyjson_mut_val* percent = yyjson_mut_obj(doc);
        if (options->percentType != defaultOptions.percentType)
            yyjson_mut_obj_add_uint(doc, percent, "type", options->percentType);
        if (options->percentNdigits != defaultOptions.percentNdigits)
            yyjson_mut_obj_add_uint(doc, percent, "ndigits", options->percentNdigits);
        {
            yyjson_mut_val* color = yyjson_mut_obj(doc);
            if (!ffStrbufEqual(&options->percentColorGreen, &defaultOptions.percentColorGreen))
                yyjson_mut_obj_add_strbuf(doc, color, "green", &options->percentColorGreen);
            if (!ffStrbufEqual(&options->percentColorYellow, &defaultOptions.percentColorYellow))
                yyjson_mut_obj_add_strbuf(doc, color, "yellow", &options->percentColorYellow);
            if (!ffStrbufEqual(&options->percentColorRed, &defaultOptions.percentColorRed))
                yyjson_mut_obj_add_strbuf(doc, color, "red", &options->percentColorRed);
            if (yyjson_mut_obj_size(color) > 0)
                yyjson_mut_obj_add_val(doc, percent, "color", color);
        }
        if (yyjson_mut_obj_size(percent) > 0)
            yyjson_mut_obj_add_val(doc, obj, "percent", percent);
    }

    {
        yyjson_mut_val* bar = yyjson_mut_obj(doc);
        if (!ffStrbufEqual(&options->barCharElapsed, &defaultOptions.barCharElapsed))
            yyjson_mut_obj_add_strbuf(doc, bar, "charElapsed", &options->barCharElapsed);
        if (!ffStrbufEqual(&options->barCharTotal, &defaultOptions.barCharTotal))
            yyjson_mut_obj_add_strbuf(doc, bar, "charTotal", &options->barCharTotal);
        if (options->barBorder != defaultOptions.barBorder)
            yyjson_mut_obj_add_bool(doc, bar, "border", options->barBorder);
        if (options->barWidth != defaultOptions.barWidth)
            yyjson_mut_obj_add_uint(doc, bar, "width", options->barWidth);

        if (yyjson_mut_obj_size(bar) > 0)
            yyjson_mut_obj_add_val(doc, obj, "bar", bar);
    }

    if (options->noBuffer != defaultOptions.noBuffer)
        yyjson_mut_obj_add_bool(doc, obj, "noBuffer", options->noBuffer);

    if (options->keyWidth != defaultOptions.keyWidth)
        yyjson_mut_obj_add_uint(doc, obj, "keyWidth", options->keyWidth);

    if (yyjson_mut_obj_size(obj) > 0)
        yyjson_mut_obj_add_val(doc, doc->root, "display", obj);
}

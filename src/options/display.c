#include "fastfetch.h"
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
        else if (ffStrEqualsIgnCase(key, "sizeNdigits"))
            options->sizeNdigits = (uint8_t) yyjson_get_uint(val);
        else if (ffStrEqualsIgnCase(key, "sizeMaxPrefix"))
        {
            int value;
            const char* error = ffJsonConfigParseEnum(val, &value, (FFKeyValuePair[]) {
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
        else if (ffStrEqualsIgnCase(key, "temperatureUnit"))
        {
            int value;
            const char* error = ffJsonConfigParseEnum(val, &value, (FFKeyValuePair[]) {
                { "CELSIUS", FF_TEMPERATURE_UNIT_CELSIUS },
                { "C", FF_TEMPERATURE_UNIT_CELSIUS },
                { "FAHRENHEIT", FF_TEMPERATURE_UNIT_FAHRENHEIT },
                { "F", FF_TEMPERATURE_UNIT_FAHRENHEIT },
                { "KELVIN", FF_TEMPERATURE_UNIT_KELVIN },
                { "K", FF_TEMPERATURE_UNIT_KELVIN },
                {},
            });
            if (error) return error;
            options->temperatureUnit = (FFTemperatureUnit) value;
        }
        else if (ffStrEqualsIgnCase(key, "percentType"))
            options->percentType = (uint8_t) yyjson_get_uint(val);
        else if (ffStrEqualsIgnCase(key, "percentNdigits"))
            options->percentNdigits = (uint8_t) yyjson_get_uint(val);
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
    else if(ffStrEqualsIgnCase(key, "--temperature-unit"))
    {
        options->temperatureUnit = (FFTemperatureUnit) ffOptionParseEnum(key, value, (FFKeyValuePair[]) {
            { "CELSIUS", FF_TEMPERATURE_UNIT_CELSIUS },
            { "C", FF_TEMPERATURE_UNIT_CELSIUS },
            { "FAHRENHEIT", FF_TEMPERATURE_UNIT_FAHRENHEIT },
            { "F", FF_TEMPERATURE_UNIT_FAHRENHEIT },
            { "KELVIN", FF_TEMPERATURE_UNIT_KELVIN },
            { "K", FF_TEMPERATURE_UNIT_KELVIN },
            {},
        });
    }
    else if(ffStrEqualsIgnCase(key, "--percent-type"))
        options->percentType = (uint8_t) ffOptionParseUInt32(key, value);
    else if(ffStrEqualsIgnCase(key, "--percent-ndigits"))
        options->percentNdigits = (uint8_t) ffOptionParseUInt32(key, value);
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
    else
        return false;
    return true;
}

void ffOptionsInitDisplay(FFOptionsDisplay* options)
{
    ffStrbufInit(&options->colorKeys);
    ffStrbufInit(&options->colorTitle);
    options->brightColor = true;
    ffStrbufInitStatic(&options->keyValueSeparator, ": ");

    options->showErrors = false;
    options->pipe = !isatty(STDOUT_FILENO);

    #ifdef NDEBUG
    options->disableLinewrap = !options->pipe;
    options->hideCursor = !options->pipe;
    #else
    options->disableLinewrap = false;
    options->hideCursor = false;
    #endif

    options->binaryPrefixType = FF_BINARY_PREFIX_TYPE_IEC;
    options->sizeNdigits = 2;
    options->sizeMaxPrefix = UINT8_MAX;
    options->temperatureUnit = FF_TEMPERATURE_UNIT_CELSIUS;
    options->stat = false;
    options->noBuffer = false;
    options->keyWidth = 0;

    ffStrbufInitStatic(&options->barCharElapsed, "■");
    ffStrbufInitStatic(&options->barCharTotal, "-");
    options->barWidth = 10;
    options->barBorder = true;
    options->percentType = 1;
    options->percentNdigits = 0;
}

void ffOptionsDestroyDisplay(FFOptionsDisplay* options)
{
    ffStrbufDestroy(&options->colorKeys);
    ffStrbufDestroy(&options->colorTitle);
    ffStrbufDestroy(&options->keyValueSeparator);
    ffStrbufDestroy(&options->barCharElapsed);
    ffStrbufDestroy(&options->barCharTotal);
}

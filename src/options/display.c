#include "fastfetch.h"
#include "common/color.h"
#include "common/jsonconfig.h"
#include "common/percent.h"
#include "util/stringUtils.h"
#include "options/display.h"

#include <unistd.h>

const char* ffOptionsParseDisplayJsonConfig(FFOptionsDisplay* options, yyjson_val* root)
{
    yyjson_val* object = yyjson_obj_get(root, "display");
    if (!object) return NULL;
    if (!yyjson_is_obj(object)) return "Property 'display' must be an object";

    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(object, idx, max, key, val)
    {
        if (unsafe_yyjson_equals_str(key, "stat"))
        {
            if (yyjson_is_bool(val))
            {
                if (yyjson_get_bool(val))
                {
                    options->stat = 0;
                    options->showErrors = true;
                }
                else
                    options->stat = -1;
            }
            else if (yyjson_is_uint(val))
            {
                options->stat = (int) yyjson_get_uint(val);
                options->showErrors = true;
            }
            else
                return "display.stat must be a boolean or a positive integer";
        }
        else if (unsafe_yyjson_equals_str(key, "pipe"))
            options->pipe = yyjson_get_bool(val);
        else if (unsafe_yyjson_equals_str(key, "showErrors"))
            options->showErrors = yyjson_get_bool(val);
        else if (unsafe_yyjson_equals_str(key, "disableLinewrap"))
            options->disableLinewrap = yyjson_get_bool(val);
        else if (unsafe_yyjson_equals_str(key, "hideCursor"))
            options->hideCursor = yyjson_get_bool(val);
        else if (unsafe_yyjson_equals_str(key, "separator"))
            ffStrbufSetJsonVal(&options->keyValueSeparator, val);
        else if (unsafe_yyjson_equals_str(key, "color"))
        {
            if (yyjson_is_str(val))
            {
                ffOptionParseColor(unsafe_yyjson_get_str(val), &options->colorKeys);
                ffStrbufSet(&options->colorTitle, &options->colorKeys);
            }
            else if (yyjson_is_obj(val))
            {
                yyjson_val* colorKeys = yyjson_obj_get(val, "keys");
                if (colorKeys)
                    ffOptionParseColor(yyjson_get_str(colorKeys), &options->colorKeys);
                yyjson_val* colorTitle = yyjson_obj_get(val, "title");
                if (colorTitle)
                    ffOptionParseColor(yyjson_get_str(colorTitle), &options->colorTitle);
                yyjson_val* colorOutput = yyjson_obj_get(val, "output");
                if (colorOutput)
                    ffOptionParseColor(yyjson_get_str(colorOutput), &options->colorOutput);
                yyjson_val* colorSeparator = yyjson_obj_get(val, "separator");
                if (colorSeparator)
                    ffOptionParseColor(yyjson_get_str(colorSeparator), &options->colorSeparator);
            }
            else
                return "display.color must be either a string or an object";
        }
        else if (unsafe_yyjson_equals_str(key, "brightColor"))
            options->brightColor = yyjson_get_bool(val);
        else if (unsafe_yyjson_equals_str(key, "duration"))
        {
            if (!yyjson_is_obj(val))
                return "display.duration must be an object";

            yyjson_val* abbreviation = yyjson_obj_get(val, "abbreviation");
            if (abbreviation) options->durationAbbreviation = yyjson_get_bool(abbreviation);

            yyjson_val* spaceBeforeUnit = yyjson_obj_get(val, "spaceBeforeUnit");
            if (spaceBeforeUnit)
            {
                int value;
                const char* error = ffJsonConfigParseEnum(spaceBeforeUnit, &value, (FFKeyValuePair[]) {
                    { "default", FF_SPACE_BEFORE_UNIT_DEFAULT },
                    { "always", FF_SPACE_BEFORE_UNIT_ALWAYS },
                    { "never", FF_SPACE_BEFORE_UNIT_NEVER },
                    {},
                });
                if (error) return error;
                options->durationSpaceBeforeUnit = (FFSpaceBeforeUnitType) value;
            }
        }
        else if (unsafe_yyjson_equals_str(key, "size"))
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

            yyjson_val* binaryPrefix = yyjson_obj_get(val, "binaryPrefix");
            if (binaryPrefix)
            {
                int value;
                const char* error = ffJsonConfigParseEnum(binaryPrefix, &value, (FFKeyValuePair[]) {
                    { "iec", FF_SIZE_BINARY_PREFIX_TYPE_IEC },
                    { "si", FF_SIZE_BINARY_PREFIX_TYPE_SI },
                    { "jedec", FF_SIZE_BINARY_PREFIX_TYPE_JEDEC },
                    {},
                });
                if (error) return error;
                options->sizeBinaryPrefix = (FFSizeBinaryPrefixType) value;
            }

            yyjson_val* ndigits = yyjson_obj_get(val, "ndigits");
            if (ndigits)
            {
                if (!yyjson_is_uint(ndigits))
                    return "display.size.ndigits must be an unsigned integer";
                uint64_t val = yyjson_get_uint(ndigits);
                if (val > 9)
                    return "display.size.ndigits must be between 0 and 9";
                options->sizeNdigits = (uint8_t) val;
            }

            yyjson_val* spaceBeforeUnit = yyjson_obj_get(val, "spaceBeforeUnit");
            if (spaceBeforeUnit)
            {
                int value;
                const char* error = ffJsonConfigParseEnum(spaceBeforeUnit, &value, (FFKeyValuePair[]) {
                    { "default", FF_SPACE_BEFORE_UNIT_DEFAULT },
                    { "always", FF_SPACE_BEFORE_UNIT_ALWAYS },
                    { "never", FF_SPACE_BEFORE_UNIT_NEVER },
                    {},
                });
                if (error) return error;
                options->sizeSpaceBeforeUnit = (FFSpaceBeforeUnitType) value;
            }
        }
        else if (unsafe_yyjson_equals_str(key, "temp"))
        {
            if (!yyjson_is_obj(val))
                return "display.temp must be an object";

            yyjson_val* unit = yyjson_obj_get(val, "unit");
            if (unit)
            {
                int value;
                const char* error = ffJsonConfigParseEnum(unit, &value, (FFKeyValuePair[]) {
                    { "DEFAULT", FF_TEMPERATURE_UNIT_DEFAULT },
                    { "D", FF_TEMPERATURE_UNIT_DEFAULT },
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
            if (ndigits)
            {
                if (!yyjson_is_uint(ndigits))
                    return "display.temperature.ndigits must be an unsigned integer";
                uint64_t val = yyjson_get_uint(ndigits);
                if (val > 9)
                    return "display.temperature.ndigits must be between 0 and 9";
                options->tempNdigits = (uint8_t) val;
            }

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

            yyjson_val* spaceBeforeUnit = yyjson_obj_get(val, "spaceBeforeUnit");
            if (spaceBeforeUnit)
            {
                int value;
                const char* error = ffJsonConfigParseEnum(spaceBeforeUnit, &value, (FFKeyValuePair[]) {
                    { "default", FF_SPACE_BEFORE_UNIT_DEFAULT },
                    { "always", FF_SPACE_BEFORE_UNIT_ALWAYS },
                    { "never", FF_SPACE_BEFORE_UNIT_NEVER },
                    {},
                });
                if (error) return error;
                options->tempSpaceBeforeUnit = (FFSpaceBeforeUnitType) value;
            }
        }
        else if (unsafe_yyjson_equals_str(key, "percent"))
        {
            if (!yyjson_is_obj(val))
                return "display.percent must be an object";

            yyjson_val* type = yyjson_obj_get(val, "type");
            if (type)
            {
                const char* error = ffPercentParseTypeJsonConfig(type, &options->percentType);
                if (error) return error;
            }

            yyjson_val* ndigits = yyjson_obj_get(val, "ndigits");
            if (ndigits)
            {
                if (!yyjson_is_uint(ndigits))
                    return "display.percent.ndigits must be an unsigned integer";
                uint64_t val = yyjson_get_uint(ndigits);
                if (val > 9)
                    return "display.percent.ndigits must be between 0 and 9";
                options->percentNdigits = (uint8_t) val;
            }

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

            yyjson_val* spaceBeforeUnit = yyjson_obj_get(val, "spaceBeforeUnit");
            if (spaceBeforeUnit)
            {
                int value;
                const char* error = ffJsonConfigParseEnum(spaceBeforeUnit, &value, (FFKeyValuePair[]) {
                    { "default", FF_SPACE_BEFORE_UNIT_DEFAULT },
                    { "always", FF_SPACE_BEFORE_UNIT_ALWAYS },
                    { "never", FF_SPACE_BEFORE_UNIT_NEVER },
                    {},
                });
                if (error) return error;
                options->percentSpaceBeforeUnit = (FFSpaceBeforeUnitType) value;
            }

            yyjson_val* width = yyjson_obj_get(val, "width");
            if (width) options->percentWidth = (uint8_t) yyjson_get_uint(width);
        }
        else if (unsafe_yyjson_equals_str(key, "bar"))
        {
            if (yyjson_is_obj(val))
            {
                yyjson_val* char_ = yyjson_obj_get(val, "char");
                if (char_)
                {
                    if (!yyjson_is_obj(char_)) return "display.bar.char must be an object";

                    yyjson_val* charElapsed = yyjson_obj_get(char_, "elapsed");
                    if (charElapsed)
                        ffStrbufSetJsonVal(&options->barCharElapsed, charElapsed);

                    yyjson_val* charTotal = yyjson_obj_get(char_, "total");
                    if (charTotal)
                        ffStrbufSetJsonVal(&options->barCharTotal, charTotal);
                }
                else
                {
                    yyjson_val* charElapsed = yyjson_obj_get(val, "charElapsed");
                    if (charElapsed)
                        return "display.bar.charElapsed has been renamed to display.bar.char.elapsed.";

                    yyjson_val* charTotal = yyjson_obj_get(val, "charTotal");
                    if (charTotal)
                        return "display.bar.charTotal has been renamed to display.bar.char.total.";
                }

                yyjson_val* border = yyjson_obj_get(val, "border");
                if (border)
                {
                    if (yyjson_is_null(border))
                    {
                        ffStrbufClear(&options->barBorderLeft);
                        ffStrbufClear(&options->barBorderRight);
                        ffStrbufClear(&options->barBorderLeftElapsed);
                        ffStrbufClear(&options->barBorderRightElapsed);
                    }
                    else
                    {
                        if (!yyjson_is_obj(border)) return "display.bar.border must be an object";

                        yyjson_val* borderLeft = yyjson_obj_get(border, "left");
                        if (borderLeft)
                            ffStrbufSetJsonVal(&options->barBorderLeft, borderLeft);

                        yyjson_val* borderRight = yyjson_obj_get(border, "right");
                        if (borderRight)
                            ffStrbufSetJsonVal(&options->barBorderRight, borderRight);

                        yyjson_val* borderLeftElapsed = yyjson_obj_get(border, "leftElapsed");
                        if (borderLeftElapsed)
                            ffStrbufSetJsonVal(&options->barBorderLeftElapsed, borderLeftElapsed);

                        yyjson_val* borderRightElapsed = yyjson_obj_get(border, "rightElapsed");
                        if (borderRightElapsed)
                            ffStrbufSetJsonVal(&options->barBorderRightElapsed, borderRightElapsed);
                    }
                }
                else
                {
                    yyjson_val* borderLeft = yyjson_obj_get(val, "borderLeft");
                    if (borderLeft)
                        return "display.bar.borderLeft has been renamed to display.bar.border.left.";

                    yyjson_val* borderRight = yyjson_obj_get(val, "borderRight");
                    if (borderRight)
                        return "display.bar.borderRight has been renamed to display.bar.border.right.";
                }

                yyjson_val* color = yyjson_obj_get(val, "color");
                if (color)
                {
                    if (yyjson_is_null(color))
                    {
                        ffStrbufClear(&options->barColorElapsed);
                        ffStrbufClear(&options->barColorTotal);
                        ffStrbufClear(&options->barColorBorder);
                    }
                    else
                    {
                        if (!yyjson_is_obj(color)) return "display.bar.color must be an object";

                        yyjson_val* colorElapsed = yyjson_obj_get(color, "elapsed");
                        if (colorElapsed)
                        {
                            const char* value = yyjson_get_str(colorElapsed);
                            if (!value)
                                ffStrbufClear(&options->barColorElapsed);
                            else if (ffStrEqualsIgnCase(value, "auto"))
                                ffStrbufSetStatic(&options->barColorElapsed, "auto");
                            else
                                ffOptionParseColor(value, &options->barColorElapsed);
                        }

                        yyjson_val* colorTotal = yyjson_obj_get(color, "total");
                        if (colorTotal) ffOptionParseColor(yyjson_get_str(colorTotal), &options->barColorTotal);

                        yyjson_val* colorBorder = yyjson_obj_get(color, "border");
                        if (colorBorder) ffOptionParseColor(yyjson_get_str(colorBorder), &options->barColorBorder);
                    }
                }

                yyjson_val* width = yyjson_obj_get(val, "width");
                if (width)
                    options->barWidth = (uint8_t) yyjson_get_uint(width);
            }
            else
                return "display.bar must be an object";
        }
        else if (unsafe_yyjson_equals_str(key, "fraction"))
        {
            if (yyjson_is_obj(val))
            {
                yyjson_val* ndigits = yyjson_obj_get(val, "ndigits");
                if (ndigits)
                {
                    if (yyjson_is_null(ndigits))
                        options->fractionNdigits = -1;
                    else
                    {
                        if (!yyjson_is_int(ndigits))
                            return "display.fraction.ndigits must be an integer";
                        int64_t val = yyjson_get_int(ndigits);
                        if (val < -1 || val > 9)
                            return "display.fraction.ndigits must be between -1 and 9";
                        options->fractionNdigits = (int8_t) val;
                    }
                }
                yyjson_val* trailingZeros = yyjson_obj_get(val, "trailingZeros");
                if (trailingZeros)
                {
                    if (yyjson_is_null(trailingZeros))
                        options->fractionTrailingZeros = FF_FRACTION_TRAILING_ZEROS_TYPE_DEFAULT;
                    else
                    {
                        int value;
                        const char* error = ffJsonConfigParseEnum(trailingZeros, &value, (FFKeyValuePair[]) {
                            { "default", FF_FRACTION_TRAILING_ZEROS_TYPE_DEFAULT },
                            { "always", FF_FRACTION_TRAILING_ZEROS_TYPE_ALWAYS },
                            { "never", FF_FRACTION_TRAILING_ZEROS_TYPE_NEVER },
                            {},
                        });
                        if (error) return error;
                        options->fractionTrailingZeros = (FFFractionTrailingZerosType) value;
                    }
                }
            }
            else
                return "display.fraction must be an object";
        }
        else if (unsafe_yyjson_equals_str(key, "noBuffer"))
            options->noBuffer = yyjson_get_bool(val);
        else if (unsafe_yyjson_equals_str(key, "key"))
        {
            if (yyjson_is_obj(val))
            {
                yyjson_val* width = yyjson_obj_get(val, "width");
                if (width)
                    options->keyWidth = (uint16_t) yyjson_get_uint(width);

                yyjson_val* type = yyjson_obj_get(val, "type");
                if (type)
                {
                    int value;
                    const char* error = ffJsonConfigParseEnum(type, &value, (FFKeyValuePair[]) {
                        { "none", FF_MODULE_KEY_TYPE_NONE },
                        { "string", FF_MODULE_KEY_TYPE_STRING },
                        { "icon", FF_MODULE_KEY_TYPE_ICON },
                        { "both", FF_MODULE_KEY_TYPE_BOTH },
                        {}
                    });
                    if (error) return error;
                    options->keyType = (uint8_t) value;
                }

                yyjson_val* paddingLeft = yyjson_obj_get(val, "paddingLeft");
                if (paddingLeft)
                    options->keyPaddingLeft = (uint16_t) yyjson_get_uint(paddingLeft);
            }
            else
                return "display.key must be an object";
        }
        else if (unsafe_yyjson_equals_str(key, "constants"))
        {
            if (!yyjson_is_arr(val))
                return "display.constants must be an array";
            yyjson_val* item;
            size_t idx, max;
            yyjson_arr_foreach(val, idx, max, item)
                ffStrbufInitJsonVal(ffListAdd(&options->constants), item);
        }
        else if (unsafe_yyjson_equals_str(key, "freq"))
        {
            if (!yyjson_is_obj(val))
                return "display.freq must be an object";

            yyjson_val* ndigits = yyjson_obj_get(val, "ndigits");
            if (ndigits)
            {
                if (yyjson_is_null(ndigits))
                    options->freqNdigits = -1;
                else
                {
                    if (!yyjson_is_int(ndigits))
                        return "display.freq.ndigits must be an integer";
                    int64_t val = yyjson_get_int(ndigits);
                    if (val < -1 || val > 9)
                        return "display.freq.ndigits must be between -1 and 9";
                    options->freqNdigits = (int8_t) val;
                }
            }

            yyjson_val* spaceBeforeUnit = yyjson_obj_get(val, "spaceBeforeUnit");
            if (spaceBeforeUnit)
            {
                int value;
                const char* error = ffJsonConfigParseEnum(spaceBeforeUnit, &value, (FFKeyValuePair[]) {
                    { "default", FF_SPACE_BEFORE_UNIT_DEFAULT },
                    { "always", FF_SPACE_BEFORE_UNIT_ALWAYS },
                    { "never", FF_SPACE_BEFORE_UNIT_NEVER },
                    {},
                });
                if (error) return error;
                options->freqSpaceBeforeUnit = (FFSpaceBeforeUnitType) value;
            }
        }
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
        if(ffOptionParseBoolean(value))
        {
            options->stat = 0;
            options->showErrors = true;
        }
        else if (value)
        {
            char* end;
            uint32_t num = (uint32_t) strtoul(value, &end, 10);
            if (*end == '\0')
            {
                options->stat = (int32_t) num;
                options->showErrors = true;
            }
            else
                options->stat = -1;
        }
        else
            options->stat = -1;
    }
    else if(ffStrEqualsIgnCase(key, "--pipe"))
        options->pipe = ffOptionParseBoolean(value);
    else if(ffStrEqualsIgnCase(key, "--show-errors"))
        options->showErrors = ffOptionParseBoolean(value);
    else if(ffStrEqualsIgnCase(key, "--debug"))
    #ifndef NDEBUG
        options->debugMode = ffOptionParseBoolean(value);
    #else
    {
        fprintf(stderr, "--debug is only available in debug builds\n");
        exit(477);
    }
    #endif
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
    else if(ffStrStartsWithIgnCase(key, "--key-"))
    {
        const char* subkey = key + strlen("--key-");
        if(ffStrEqualsIgnCase(subkey, "width"))
            options->keyWidth = (uint16_t) ffOptionParseUInt32(key, value);
        else if(ffStrEqualsIgnCase(subkey, "type"))
        {
            options->keyType = (FFModuleKeyType) ffOptionParseEnum(key, value, (FFKeyValuePair[]) {
                { "none", FF_MODULE_KEY_TYPE_NONE },
                { "string", FF_MODULE_KEY_TYPE_STRING },
                { "icon", FF_MODULE_KEY_TYPE_ICON },
                { "both", FF_MODULE_KEY_TYPE_BOTH },
                {}
            });
        }
        else if(ffStrEqualsIgnCase(subkey, "padding-left"))
            options->keyPaddingLeft = (uint16_t) ffOptionParseUInt32(key, value);
        else
            return false;
    }
    else if(ffStrEqualsIgnCase(key, "--bright-color"))
        options->brightColor = ffOptionParseBoolean(value);
    else if(ffStrEqualsIgnCase(key, "--binary-prefix"))
    {
        fprintf(stderr, "--binary-prefix has been renamed to --size-binary-prefix\n");
        exit(477);
    }
    else if(ffStrStartsWithIgnCase(key, "--duration-"))
    {
        const char* subkey = key + strlen("--duration-");
        if(ffStrEqualsIgnCase(subkey, "abbreviation"))
            options->durationAbbreviation = ffOptionParseBoolean(value);
        else if(ffStrEqualsIgnCase(subkey, "space-before-unit"))
        {
            options->durationSpaceBeforeUnit = (FFSpaceBeforeUnitType) ffOptionParseEnum(key, value, (FFKeyValuePair[]) {
                { "default", FF_SPACE_BEFORE_UNIT_DEFAULT },
                { "always", FF_SPACE_BEFORE_UNIT_ALWAYS },
                { "never", FF_SPACE_BEFORE_UNIT_NEVER },
                {},
            });
        }
        else
            return false;
    }
    else if(ffStrStartsWithIgnCase(key, "--size-"))
    {
        const char* subkey = key + strlen("--size-");
        if (ffStrEqualsIgnCase(subkey, "binary-prefix"))
        {
            options->sizeBinaryPrefix = (FFSizeBinaryPrefixType) ffOptionParseEnum(key, value, (FFKeyValuePair[]) {
                { "iec", FF_SIZE_BINARY_PREFIX_TYPE_IEC },
                { "si", FF_SIZE_BINARY_PREFIX_TYPE_SI },
                { "jedec", FF_SIZE_BINARY_PREFIX_TYPE_JEDEC },
                {}
            });
        }
        else if (ffStrEqualsIgnCase(subkey, "ndigits"))
            options->sizeNdigits = (uint8_t) ffOptionParseUInt32(key, value);
        else if (ffStrEqualsIgnCase(subkey, "max-prefix"))
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
        else if(ffStrEqualsIgnCase(subkey, "space-before-unit"))
        {
            options->sizeSpaceBeforeUnit = (FFSpaceBeforeUnitType) ffOptionParseEnum(key, value, (FFKeyValuePair[]) {
                { "default", FF_SPACE_BEFORE_UNIT_DEFAULT },
                { "always", FF_SPACE_BEFORE_UNIT_ALWAYS },
                { "never", FF_SPACE_BEFORE_UNIT_NEVER },
                {},
            });
        }
        else
            return false;
    }
    else if(ffStrStartsWithIgnCase(key, "--temp-"))
    {
        const char* subkey = key + strlen("--temp-");
        if(ffStrEqualsIgnCase(subkey, "unit"))
        {
            options->tempUnit = (FFTemperatureUnit) ffOptionParseEnum(key, value, (FFKeyValuePair[]) {
                { "DEFAULT", FF_TEMPERATURE_UNIT_DEFAULT },
                { "D", FF_TEMPERATURE_UNIT_DEFAULT },
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
        else if(ffStrEqualsIgnCase(subkey, "space-before-unit"))
        {
            options->tempSpaceBeforeUnit = (FFSpaceBeforeUnitType) ffOptionParseEnum(key, value, (FFKeyValuePair[]) {
                { "default", FF_SPACE_BEFORE_UNIT_DEFAULT },
                { "always", FF_SPACE_BEFORE_UNIT_ALWAYS },
                { "never", FF_SPACE_BEFORE_UNIT_NEVER },
                {},
            });
        }
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
        else if(ffStrEqualsIgnCase(subkey, "space-before-unit"))
        {
            options->percentSpaceBeforeUnit = (FFSpaceBeforeUnitType) ffOptionParseEnum(key, value, (FFKeyValuePair[]) {
                { "default", FF_SPACE_BEFORE_UNIT_DEFAULT },
                { "always", FF_SPACE_BEFORE_UNIT_ALWAYS },
                { "never", FF_SPACE_BEFORE_UNIT_NEVER },
                {},
            });
        }
        else if(ffStrEqualsIgnCase(subkey, "width"))
            options->percentWidth = (uint8_t) ffOptionParseUInt32(key, value);
        else
            return false;
    }
    else if(ffStrEqualsIgnCase(key, "--fraction-ndigits"))
        options->fractionNdigits = (int8_t) ffOptionParseInt32(key, value);
    else if(ffStrEqualsIgnCase(key, "--fraction-trailing-zeros"))
    {
        options->fractionTrailingZeros = (FFFractionTrailingZerosType) ffOptionParseEnum(key, value, (FFKeyValuePair[]) {
            { "default", FF_FRACTION_TRAILING_ZEROS_TYPE_DEFAULT },
            { "always", FF_FRACTION_TRAILING_ZEROS_TYPE_ALWAYS },
            { "never", FF_FRACTION_TRAILING_ZEROS_TYPE_NEVER },
            {},
        });
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
        else if(ffStrEqualsIgnCase(subkey, "border-left"))
            ffOptionParseString(key, value, &options->barBorderLeft);
        else if(ffStrEqualsIgnCase(subkey, "border-right"))
            ffOptionParseString(key, value, &options->barBorderRight);
        else if(ffStrEqualsIgnCase(subkey, "border-left-elapsed"))
            ffOptionParseString(key, value, &options->barBorderLeftElapsed);
        else if(ffStrEqualsIgnCase(subkey, "border-right-elapsed"))
            ffOptionParseString(key, value, &options->barBorderRightElapsed);
        else if(ffStrEqualsIgnCase(subkey, "color-elapsed"))
        {
            if (!value)
                ffStrbufClear(&options->barColorElapsed);
            else if (ffStrEqualsIgnCase(value, "auto"))
                ffStrbufSetStatic(&options->barColorElapsed, "auto");
            else
                ffOptionParseColor(value, &options->barColorElapsed);
        }
        else if(ffStrEqualsIgnCase(subkey, "color-total"))
            ffOptionParseColor(value, &options->barColorTotal);
        else if(ffStrEqualsIgnCase(subkey, "color-border"))
            ffOptionParseColor(value, &options->barColorBorder);
        else
            return false;
    }
    else if(ffStrStartsWithIgnCase(key, "--freq-"))
    {
        const char* subkey = key + strlen("--freq-");
        if(ffStrEqualsIgnCase(subkey, "ndigits"))
            options->freqNdigits = (int8_t) ffOptionParseInt32(key, value);
        else if(ffStrEqualsIgnCase(subkey, "space-before-unit"))
        {
            options->freqSpaceBeforeUnit = (FFSpaceBeforeUnitType) ffOptionParseEnum(key, value, (FFKeyValuePair[]) {
                { "default", FF_SPACE_BEFORE_UNIT_DEFAULT },
                { "always", FF_SPACE_BEFORE_UNIT_ALWAYS },
                { "never", FF_SPACE_BEFORE_UNIT_NEVER },
                {},
            });
        }
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
    ffStrbufInit(&options->colorOutput);
    ffStrbufInit(&options->colorSeparator);
    options->brightColor = !instance.state.terminalLightTheme;
    ffStrbufInitStatic(&options->keyValueSeparator, ": ");

    options->showErrors = false;
    options->pipe = !isatty(STDOUT_FILENO) || !!getenv("NO_COLOR");

    #ifdef NDEBUG
    options->disableLinewrap = !options->pipe;
    #else
    options->disableLinewrap = false;
    options->debugMode = false;
    #endif

    options->durationSpaceBeforeUnit = FF_SPACE_BEFORE_UNIT_DEFAULT;
    options->hideCursor = false;
    options->sizeBinaryPrefix = FF_SIZE_BINARY_PREFIX_TYPE_IEC;
    options->sizeNdigits = 2;
    options->sizeMaxPrefix = 8; // YB
    options->sizeSpaceBeforeUnit = FF_SPACE_BEFORE_UNIT_DEFAULT;

    options->stat = -1;
    options->noBuffer = false;
    options->keyWidth = 0;
    options->keyPaddingLeft = 0;
    options->keyType = FF_MODULE_KEY_TYPE_STRING;

    options->tempUnit = FF_TEMPERATURE_UNIT_DEFAULT;
    options->tempNdigits = 1;
    ffStrbufInitStatic(&options->tempColorGreen, FF_COLOR_FG_GREEN);
    ffStrbufInitStatic(&options->tempColorYellow, instance.state.terminalLightTheme ? FF_COLOR_FG_YELLOW : FF_COLOR_FG_LIGHT_YELLOW);
    ffStrbufInitStatic(&options->tempColorRed, instance.state.terminalLightTheme ? FF_COLOR_FG_RED : FF_COLOR_FG_LIGHT_RED);
    options->tempSpaceBeforeUnit = FF_SPACE_BEFORE_UNIT_DEFAULT;

    ffStrbufInitStatic(&options->barCharElapsed, "■");
    ffStrbufInitStatic(&options->barCharTotal, "-");
    ffStrbufInitStatic(&options->barBorderLeft, "[ ");
    ffStrbufInitStatic(&options->barBorderRight, " ]");
    ffStrbufInit(&options->barBorderLeftElapsed);
    ffStrbufInit(&options->barBorderRightElapsed);
    ffStrbufInitStatic(&options->barColorElapsed, "auto");
    ffStrbufInitStatic(&options->barColorTotal, instance.state.terminalLightTheme ? FF_COLOR_FG_WHITE : FF_COLOR_FG_LIGHT_WHITE);
    ffStrbufInitStatic(&options->barColorBorder, instance.state.terminalLightTheme ? FF_COLOR_FG_WHITE : FF_COLOR_FG_LIGHT_WHITE);
    options->barWidth = 10;

    options->durationAbbreviation = false;
    options->durationSpaceBeforeUnit = FF_SPACE_BEFORE_UNIT_DEFAULT;
    options->percentType = 9;
    options->percentNdigits = 0;
    ffStrbufInitStatic(&options->percentColorGreen, FF_COLOR_FG_GREEN);
    ffStrbufInitStatic(&options->percentColorYellow, instance.state.terminalLightTheme ? FF_COLOR_FG_YELLOW : FF_COLOR_FG_LIGHT_YELLOW);
    ffStrbufInitStatic(&options->percentColorRed, instance.state.terminalLightTheme ? FF_COLOR_FG_RED : FF_COLOR_FG_LIGHT_RED);
    options->percentSpaceBeforeUnit = FF_SPACE_BEFORE_UNIT_DEFAULT;
    options->percentWidth = 0;

    options->freqNdigits = 2;
    options->freqSpaceBeforeUnit = FF_SPACE_BEFORE_UNIT_DEFAULT;
    options->fractionNdigits = 2;
    options->fractionTrailingZeros = FF_FRACTION_TRAILING_ZEROS_TYPE_DEFAULT;

    ffListInit(&options->constants, sizeof(FFstrbuf));
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
    FF_LIST_FOR_EACH(FFstrbuf, item, options->constants)
        ffStrbufDestroy(item);
    ffListDestroy(&options->constants);
}

void ffOptionsGenerateDisplayJsonConfig(FFOptionsDisplay* options, yyjson_mut_doc* doc)
{
    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, doc->root, "display");

    if (options->stat <= 0)
        yyjson_mut_obj_add_bool(doc, obj, "stat", options->stat == 0);
    else
        yyjson_mut_obj_add_int(doc, obj, "stat", options->stat);

    yyjson_mut_obj_add_bool(doc, obj, "pipe", options->pipe);

    yyjson_mut_obj_add_bool(doc, obj, "showErrors", options->showErrors);

    yyjson_mut_obj_add_bool(doc, obj, "disableLinewrap", options->disableLinewrap);

    yyjson_mut_obj_add_bool(doc, obj, "hideCursor", options->hideCursor);

    yyjson_mut_obj_add_strbuf(doc, obj, "separator", &options->keyValueSeparator);

    {
        yyjson_mut_val* color = yyjson_mut_obj_add_obj(doc, obj, "color");
        yyjson_mut_obj_add_strbuf(doc, color, "keys", &options->colorKeys);
        yyjson_mut_obj_add_strbuf(doc, color, "title", &options->colorTitle);
        yyjson_mut_obj_add_strbuf(doc, color, "output", &options->colorOutput);
        yyjson_mut_obj_add_strbuf(doc, color, "separator", &options->colorSeparator);
    }

    yyjson_mut_obj_add_bool(doc, obj, "brightColor", options->brightColor);

    {
        yyjson_mut_val* duration = yyjson_mut_obj_add_obj(doc, obj, "duration");
        yyjson_mut_obj_add_bool(doc, duration, "abbreviation", options->durationAbbreviation);
        switch (options->durationSpaceBeforeUnit)
        {
            case FF_SPACE_BEFORE_UNIT_DEFAULT:
                yyjson_mut_obj_add_str(doc, duration, "spaceBeforeUnit", "default");
                break;
            case FF_SPACE_BEFORE_UNIT_ALWAYS:
                yyjson_mut_obj_add_str(doc, duration, "spaceBeforeUnit", "always");
                break;
            case FF_SPACE_BEFORE_UNIT_NEVER:
                yyjson_mut_obj_add_str(doc, duration, "spaceBeforeUnit", "never");
                break;
        }
    }

    {
        yyjson_mut_val* size = yyjson_mut_obj_add_obj(doc, obj, "size");
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
        switch (options->sizeBinaryPrefix)
        {
            case FF_SIZE_BINARY_PREFIX_TYPE_IEC:
                yyjson_mut_obj_add_str(doc, size, "binaryPrefix", "iec");
                break;
            case FF_SIZE_BINARY_PREFIX_TYPE_SI:
                yyjson_mut_obj_add_str(doc, size, "binaryPrefix", "si");
                break;
            case FF_SIZE_BINARY_PREFIX_TYPE_JEDEC:
                yyjson_mut_obj_add_str(doc, size, "binaryPrefix", "jedec");
                break;
        }
        yyjson_mut_obj_add_uint(doc, size, "ndigits", options->sizeNdigits);
        switch (options->sizeSpaceBeforeUnit)
        {
            case FF_SPACE_BEFORE_UNIT_DEFAULT:
                yyjson_mut_obj_add_str(doc, size, "spaceBeforeUnit", "default");
                break;
            case FF_SPACE_BEFORE_UNIT_ALWAYS:
                yyjson_mut_obj_add_str(doc, size, "spaceBeforeUnit", "always");
                break;
            case FF_SPACE_BEFORE_UNIT_NEVER:
                yyjson_mut_obj_add_str(doc, size, "spaceBeforeUnit", "never");
                break;
        }
    }

    {
        yyjson_mut_val* temperature = yyjson_mut_obj_add_obj(doc, obj, "temp");
        switch (options->tempUnit)
        {
            case FF_TEMPERATURE_UNIT_DEFAULT:
                yyjson_mut_obj_add_str(doc, temperature, "unit", "D");
                break;
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
        yyjson_mut_obj_add_uint(doc, temperature, "ndigits", options->tempNdigits);
        {
            yyjson_mut_val* color = yyjson_mut_obj_add_obj(doc, temperature, "color");
            yyjson_mut_obj_add_strbuf(doc, color, "green", &options->tempColorGreen);
            yyjson_mut_obj_add_strbuf(doc, color, "yellow", &options->tempColorYellow);
            yyjson_mut_obj_add_strbuf(doc, color, "red", &options->tempColorRed);
        }
        switch (options->tempSpaceBeforeUnit)
        {
            case FF_SPACE_BEFORE_UNIT_DEFAULT:
                yyjson_mut_obj_add_str(doc, temperature, "spaceBeforeUnit", "default");
                break;
            case FF_SPACE_BEFORE_UNIT_ALWAYS:
                yyjson_mut_obj_add_str(doc, temperature, "spaceBeforeUnit", "always");
                break;
            case FF_SPACE_BEFORE_UNIT_NEVER:
                yyjson_mut_obj_add_str(doc, temperature, "spaceBeforeUnit", "never");
                break;
        }
    }

    {
        yyjson_mut_val* percent = yyjson_mut_obj_add_obj(doc, obj, "percent");
        {
            yyjson_mut_val* type = yyjson_mut_obj_add_arr(doc, percent, "type");
            if (options->percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
                yyjson_mut_arr_add_str(doc, type, "num");
            if (options->percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
                yyjson_mut_arr_add_str(doc, type, "var");
            if (options->percentType & FF_PERCENTAGE_TYPE_HIDE_OTHERS_BIT)
                yyjson_mut_arr_add_str(doc, type, "hide-others");
            if (options->percentType & FF_PERCENTAGE_TYPE_NUM_COLOR_BIT)
                yyjson_mut_arr_add_str(doc, type, "num-color");
            if (options->percentType & FF_PERCENTAGE_TYPE_BAR_MONOCHROME_BIT)
                yyjson_mut_arr_add_str(doc, type, "bar-monochrome");
        }
        yyjson_mut_obj_add_uint(doc, percent, "ndigits", options->percentNdigits);
        {
            yyjson_mut_val* color = yyjson_mut_obj_add_obj(doc, percent, "color");
            yyjson_mut_obj_add_strbuf(doc, color, "green", &options->percentColorGreen);
            yyjson_mut_obj_add_strbuf(doc, color, "yellow", &options->percentColorYellow);
            yyjson_mut_obj_add_strbuf(doc, color, "red", &options->percentColorRed);
        }
        switch (options->percentSpaceBeforeUnit)
        {
            case FF_SPACE_BEFORE_UNIT_DEFAULT:
                yyjson_mut_obj_add_str(doc, percent, "spaceBeforeUnit", "default");
                break;
            case FF_SPACE_BEFORE_UNIT_ALWAYS:
                yyjson_mut_obj_add_str(doc, percent, "spaceBeforeUnit", "always");
                break;
            case FF_SPACE_BEFORE_UNIT_NEVER:
                yyjson_mut_obj_add_str(doc, percent, "spaceBeforeUnit", "never");
                break;
        }
        yyjson_mut_obj_add_uint(doc, percent, "width", options->percentWidth);
    }

    {
        yyjson_mut_val* bar = yyjson_mut_obj_add_obj(doc, obj, "bar");

        yyjson_mut_val* char_ = yyjson_mut_obj_add_obj(doc, bar, "char");
        yyjson_mut_obj_add_strbuf(doc, char_, "elapsed", &options->barCharElapsed);
        yyjson_mut_obj_add_strbuf(doc, char_, "total", &options->barCharTotal);

        yyjson_mut_val* border = yyjson_mut_obj_add_obj(doc, bar, "border");
        yyjson_mut_obj_add_strbuf(doc, border, "left", &options->barBorderLeft);
        yyjson_mut_obj_add_strbuf(doc, border, "right", &options->barBorderRight);
        yyjson_mut_obj_add_strbuf(doc, border, "leftElapsed", &options->barBorderLeftElapsed);
        yyjson_mut_obj_add_strbuf(doc, border, "rightElapsed", &options->barBorderRightElapsed);

        yyjson_mut_val* color = yyjson_mut_obj_add_obj(doc, bar, "color");
        yyjson_mut_obj_add_strbuf(doc, color, "elapsed", &options->barColorElapsed);
        yyjson_mut_obj_add_strbuf(doc, color, "total", &options->barColorTotal);
        yyjson_mut_obj_add_strbuf(doc, color, "border", &options->barColorBorder);

        yyjson_mut_obj_add_uint(doc, bar, "width", options->barWidth);
    }

    {
        yyjson_mut_val* fraction = yyjson_mut_obj_add_obj(doc, obj, "fraction");

        if (options->fractionNdigits < 0)
            yyjson_mut_obj_add_null(doc, fraction, "ndigits");
        else
            yyjson_mut_obj_add_uint(doc, fraction, "ndigits", (uint8_t) options->fractionNdigits);
    }

    yyjson_mut_obj_add_bool(doc, obj, "noBuffer", options->noBuffer);

    {
        yyjson_mut_val* key = yyjson_mut_obj_add_obj(doc, obj, "key");
        yyjson_mut_obj_add_uint(doc, key, "width", options->keyWidth);
        switch ((uint8_t) options->keyType)
        {
            case FF_MODULE_KEY_TYPE_NONE:
                yyjson_mut_obj_add_str(doc, key, "type", "none");
                break;
            case FF_MODULE_KEY_TYPE_STRING:
                yyjson_mut_obj_add_str(doc, key, "type", "string");
                break;
            case FF_MODULE_KEY_TYPE_ICON:
                yyjson_mut_obj_add_str(doc, key, "type", "icon");
                break;
            case FF_MODULE_KEY_TYPE_BOTH:
                yyjson_mut_obj_add_str(doc, key, "type", "both");
                break;
        }

        yyjson_mut_obj_add_uint(doc, key, "paddingLeft", options->keyPaddingLeft);
    }

    {
        yyjson_mut_val* freq = yyjson_mut_obj_add_obj(doc, obj, "freq");
        yyjson_mut_obj_add_int(doc, freq, "ndigits", options->freqNdigits);
        switch (options->percentSpaceBeforeUnit)
        {
            case FF_SPACE_BEFORE_UNIT_DEFAULT:
                yyjson_mut_obj_add_str(doc, freq, "spaceBeforeUnit", "default");
                break;
            case FF_SPACE_BEFORE_UNIT_ALWAYS:
                yyjson_mut_obj_add_str(doc, freq, "spaceBeforeUnit", "always");
                break;
            case FF_SPACE_BEFORE_UNIT_NEVER:
                yyjson_mut_obj_add_str(doc, freq, "spaceBeforeUnit", "never");
                break;
        }
    }

    {
        yyjson_mut_val* constants = yyjson_mut_obj_add_arr(doc, obj, "constants");
        FF_LIST_FOR_EACH(FFstrbuf, item, options->constants)
            yyjson_mut_arr_add_strbuf(doc, constants, item);
    }
}

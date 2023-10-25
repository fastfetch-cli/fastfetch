#include "logo/logo.h"

#include "common/jsonconfig.h"
#include "util/stringUtils.h"

void ffInitLogoOptions(FFLogoOptions* options)
{
    ffStrbufInit(&options->source);
    options->type = FF_LOGO_TYPE_AUTO;
    for(uint8_t i = 0; i < (uint8_t) FASTFETCH_LOGO_MAX_COLORS; ++i)
        ffStrbufInit(&options->colors[i]);
    options->width = 0;
    options->height = 0; //preserve aspect ratio
    options->paddingTop = 0;
    options->paddingLeft = 0;
    options->paddingRight = 4;
    options->printRemaining = true;
    options->preserveAspectRadio = false;
    options->recache = false;
    options->separate = false;

    options->chafaFgOnly = false;
    ffStrbufInitStatic(&options->chafaSymbols, "block+border+space-wide-inverted"); // Chafa default
    options->chafaCanvasMode = UINT32_MAX;
    options->chafaColorSpace = UINT32_MAX;
    options->chafaDitherMode = UINT32_MAX;
}

bool ffParseLogoCommandOptions(FFLogoOptions* options, const char* key, const char* value)
{
    if (strcasecmp(key, "-l") == 0)
        goto logoType;

    const char* subKey = ffOptionTestPrefix(key, "logo");
    if(subKey)
    {
        if (subKey[0] == '\0')
        {
logoType:
            if(value == NULL)
            {
                fprintf(stderr, "Error: usage: %s <none|small|logo-source>\n", key);
                exit(477);
            }
            //this is usually wanted when disabling logo
            if(strcasecmp(value, "none") == 0)
            {
                options->paddingTop = 0;
                options->paddingRight = 0;
                options->paddingLeft = 0;
                options->type = FF_LOGO_TYPE_NONE;
            }
            else if(strcasecmp(value, "small") == 0)
            {
                options->type = FF_LOGO_TYPE_SMALL;
            }
            else
                ffOptionParseString(key, value, &options->source);
        }
        else if(strcasecmp(subKey, "type") == 0)
        {
            options->type = (FFLogoType) ffOptionParseEnum(key, value, (FFKeyValuePair[]) {
                { "auto", FF_LOGO_TYPE_AUTO },
                { "builtin", FF_LOGO_TYPE_BUILTIN },
                { "small", FF_LOGO_TYPE_SMALL },
                { "file", FF_LOGO_TYPE_FILE },
                { "file-raw", FF_LOGO_TYPE_FILE_RAW },
                { "data", FF_LOGO_TYPE_DATA },
                { "data-raw", FF_LOGO_TYPE_DATA_RAW },
                { "sixel", FF_LOGO_TYPE_IMAGE_SIXEL },
                { "kitty", FF_LOGO_TYPE_IMAGE_KITTY },
                { "kitty-direct", FF_LOGO_TYPE_IMAGE_KITTY_DIRECT },
                { "iterm", FF_LOGO_TYPE_IMAGE_ITERM },
                { "chafa", FF_LOGO_TYPE_IMAGE_CHAFA },
                { "raw", FF_LOGO_TYPE_IMAGE_RAW },
                { "none", FF_LOGO_TYPE_NONE },
                {},
            });
        }
        else if(ffStrStartsWithIgnCase(subKey, "color-") && subKey[6] != '\0' && subKey[7] == '\0') // matches "--logo-color-*"
        {
            //Map the number to an array index, so that '1' -> 0, '2' -> 1, etc.
            int index = (int)subKey[6] - '0' - 1;

            //Match only --logo-color-[1-9]
            if(index < 0 || index >= FASTFETCH_LOGO_MAX_COLORS)
            {
                fprintf(stderr, "Error: invalid --color-[1-9] index: %c\n", key[13]);
                exit(472);
            }
            if(value == NULL)
            {
                fprintf(stderr, "Error: usage: %s <str>\n", key);
                exit(477);
            }
            ffOptionParseColor(value, &options->colors[index]);
        }
        else if(strcasecmp(subKey, "width") == 0)
            options->width = ffOptionParseUInt32(key, value);
        else if(strcasecmp(subKey, "height") == 0)
            options->height = ffOptionParseUInt32(key, value);
        else if(strcasecmp(subKey, "padding") == 0)
        {
            uint32_t padding = ffOptionParseUInt32(key, value);
            options->paddingLeft = padding;
            options->paddingRight = padding;
        }
        else if(strcasecmp(subKey, "padding-top") == 0)
            options->paddingTop = ffOptionParseUInt32(key, value);
        else if(strcasecmp(subKey, "padding-left") == 0)
            options->paddingLeft = ffOptionParseUInt32(key, value);
        else if(strcasecmp(subKey, "padding-right") == 0)
            options->paddingRight = ffOptionParseUInt32(key, value);
        else if(strcasecmp(subKey, "print-remaining") == 0)
            options->printRemaining = ffOptionParseBoolean(value);
        else if(strcasecmp(subKey, "preserve-aspect-radio") == 0)
            options->preserveAspectRadio = ffOptionParseBoolean(value);
        else if(strcasecmp(subKey, "recache") == 0)
            options->recache = ffOptionParseBoolean(value);
        else if(strcasecmp(subKey, "separate") == 0)
            options->separate = ffOptionParseBoolean(value);
        else
            return false;
    }
    else if((subKey = ffOptionTestPrefix(key, "file")))
    {
        if(subKey[0] == '\0')
        {
            ffOptionParseString(key, value, &options->source);
            options->type = FF_LOGO_TYPE_FILE;
        }
        else if(strcasecmp(subKey, "raw") == 0)
        {
            ffOptionParseString(key, value, &options->source);
            options->type = FF_LOGO_TYPE_FILE_RAW;
        }
        else
            return false;
    }
    else if((subKey = ffOptionTestPrefix(key, "data")))
    {
        if(subKey[0] == '\0')
        {
            ffOptionParseString(key, value, &options->source);
            options->type = FF_LOGO_TYPE_DATA;
        }
        else if(strcasecmp(subKey, "raw") == 0)
        {
            ffOptionParseString(key, value, &options->source);
            options->type = FF_LOGO_TYPE_DATA_RAW;
        }
        else
            return false;
    }
    else if(strcasecmp(key, "--sixel") == 0)
    {
        ffOptionParseString(key, value, &options->source);
        options->type = FF_LOGO_TYPE_IMAGE_SIXEL;
    }
    else if(strcasecmp(key, "--kitty") == 0)
    {
        ffOptionParseString(key, value, &options->source);
        options->type = FF_LOGO_TYPE_IMAGE_KITTY;
    }
    else if(strcasecmp(key, "--kitty-direct") == 0)
    {
        ffOptionParseString(key, value, &options->source);
        options->type = FF_LOGO_TYPE_IMAGE_KITTY_DIRECT;
    }
    else if(strcasecmp(key, "--iterm") == 0)
    {
        ffOptionParseString(key, value, &options->source);
        options->type = FF_LOGO_TYPE_IMAGE_ITERM;
    }
    else if(strcasecmp(key, "--raw") == 0)
    {
        ffOptionParseString(key, value, &options->source);
        options->type = FF_LOGO_TYPE_IMAGE_RAW;
    }
    else if((subKey = ffOptionTestPrefix(key, "chafa")))
    {
        if(subKey[0] == '\0')
        {
            ffOptionParseString(key, value, &options->source);
            options->type = FF_LOGO_TYPE_IMAGE_CHAFA;
        }
        else if(strcasecmp(subKey, "fg-only") == 0)
            options->chafaFgOnly = ffOptionParseBoolean(value);
        else if(strcasecmp(subKey, "symbols") == 0)
            ffOptionParseString(key, value, &options->chafaSymbols);
        else if(strcasecmp(subKey, "canvas-mode") == 0)
            options->chafaCanvasMode = ffOptionParseUInt32(key, value);
        else if(strcasecmp(subKey, "color-space") == 0)
            options->chafaColorSpace = ffOptionParseUInt32(key, value);
        else if(strcasecmp(subKey, "dither-mode") == 0)
            options->chafaDitherMode = ffOptionParseUInt32(key, value);
        else
            return false;
    }
    else
        return false;

    return true;
}

void ffDestroyLogoOptions(FFLogoOptions* options)
{
    ffStrbufDestroy(&options->source);
    ffStrbufDestroy(&options->chafaSymbols);
    for(uint8_t i = 0; i < (uint8_t) FASTFETCH_LOGO_MAX_COLORS; ++i)
        ffStrbufDestroy(&options->colors[i]);
}

const char* ffParseLogoJsonConfig(FFLogoOptions* options, yyjson_val* root)
{
    yyjson_val* object = yyjson_obj_get(root, "logo");
    if (!object) return NULL;
    if (yyjson_is_null(object))
    {
        options->type = FF_LOGO_TYPE_NONE;
        options->paddingTop = 0;
        options->paddingRight = 0;
        options->paddingLeft = 0;
        return NULL;
    }

    if (yyjson_is_str(object))
    {
        const char* value = yyjson_get_str(object);
        ffStrbufSetS(&options->source, value);
        return NULL;
    }

    if (!yyjson_is_obj(object)) return "Property 'logo' must be an object";

    yyjson_val *key_, *val;
    size_t idx, max;
    yyjson_obj_foreach(object, idx, max, key_, val)
    {
        const char* key = yyjson_get_str(key_);

        if (strcasecmp(key, "type") == 0)
        {
            int value;
            const char* error = ffJsonConfigParseEnum(val, &value, (FFKeyValuePair[]) {
                { "auto", FF_LOGO_TYPE_AUTO },
                { "builtin", FF_LOGO_TYPE_BUILTIN },
                { "small", FF_LOGO_TYPE_SMALL },
                { "file", FF_LOGO_TYPE_FILE },
                { "file-raw", FF_LOGO_TYPE_FILE_RAW },
                { "data", FF_LOGO_TYPE_DATA },
                { "data-raw", FF_LOGO_TYPE_DATA_RAW },
                { "sixel", FF_LOGO_TYPE_IMAGE_SIXEL },
                { "kitty", FF_LOGO_TYPE_IMAGE_KITTY },
                { "kitty-direct", FF_LOGO_TYPE_IMAGE_KITTY_DIRECT },
                { "iterm", FF_LOGO_TYPE_IMAGE_ITERM },
                { "chafa", FF_LOGO_TYPE_IMAGE_CHAFA },
                { "raw", FF_LOGO_TYPE_IMAGE_RAW },
                { "none", FF_LOGO_TYPE_NONE },
                {},
            });

            if (error) return error;
            options->type = (FFLogoType) value;
            continue;
        }
        else if (strcasecmp(key, "source") == 0)
        {
            ffStrbufSetS(&options->source, yyjson_get_str(val));
            continue;
        }
        else if (strcasecmp(key, "color") == 0)
        {
            if (!yyjson_is_obj(val))
                return "Property 'color' must be an object";

            yyjson_val *key_c, *valc;
            size_t idxc, maxc;
            yyjson_obj_foreach(val, idxc, maxc, key_c, valc)
            {
                const char* keyc = yyjson_get_str(key_c);
                uint32_t index = (uint32_t) strtoul(keyc, NULL, 10);
                if (index < 1 || index > FASTFETCH_LOGO_MAX_COLORS)
                    return "Keys of property 'color' must be a number between 1 to 9";

                ffOptionParseColor(yyjson_get_str(valc), &options->colors[index - 1]);
            }
            continue;
        }
        else if (strcasecmp(key, "width") == 0)
        {
            uint32_t value = (uint32_t) yyjson_get_uint(val);
            if (value == 0)
                return "Logo width must be a possitive integer";
            options->width = value;
            continue;
        }
        else if (strcasecmp(key, "height") == 0)
        {
            uint32_t value = (uint32_t) yyjson_get_uint(val);
            if (value == 0)
                return "Logo height must be a possitive integer";
            options->height = value;
            continue;
        }
        else if (strcasecmp(key, "padding") == 0)
        {
            if (!yyjson_is_obj(val))
                return "Logo padding must be an object";

            #define FF_PARSE_PADDING_POSITON(pos, paddingPos) \
                yyjson_val* pos = yyjson_obj_get(val, #pos); \
                if (pos) \
                { \
                    if (!yyjson_is_uint(pos)) \
                        return "Logo padding values must be possitive integers"; \
                    options->paddingPos = (uint32_t) yyjson_get_uint(pos); \
                }
            FF_PARSE_PADDING_POSITON(left, paddingLeft);
            FF_PARSE_PADDING_POSITON(top, paddingTop);
            FF_PARSE_PADDING_POSITON(right, paddingRight);
            #undef FF_PARSE_PADDING_POSITON
            continue;
        }
        else if (strcasecmp(key, "printRemaining") == 0)
        {
            options->printRemaining = yyjson_get_bool(val);
            continue;
        }
        else if (strcasecmp(key, "preserveAspectRadio") == 0)
        {
            options->preserveAspectRadio = yyjson_get_bool(val);
            continue;
        }
        else if (strcasecmp(key, "recache") == 0)
        {
            options->recache = yyjson_get_bool(val);
            continue;
        }
        else if (strcasecmp(key, "separate") == 0)
        {
            options->separate = yyjson_get_bool(val);
            continue;
        }
        else if (strcasecmp(key, "chafa") == 0)
        {
            if (!yyjson_is_obj(val))
                return "Chafa config must be an object";

            yyjson_val* fgOnly = yyjson_obj_get(val, "fgOnly");
            if (fgOnly)
                options->chafaFgOnly = yyjson_get_bool(fgOnly);

            yyjson_val* symbols = yyjson_obj_get(val, "symbols");
            if (symbols)
                ffStrbufAppendS(&options->chafaSymbols, yyjson_get_str(symbols));

            yyjson_val* canvasMode = yyjson_obj_get(val, "canvasMode");
            if (canvasMode)
            {
                int value;
                const char* error = ffJsonConfigParseEnum(canvasMode, &value, (FFKeyValuePair[]) {
                    { "TRUECOLOR", 0 },
                    { "INDEXED_256", 1 },
                    { "INDEXED_240", 2 },
                    { "INDEXED_16", 3 },
                    { "FGBG_BGFG", 4 },
                    { "FGBG", 5 },
                    { "INDEXED_8", 6 },
                    { "INDEXED_16_8", 7 },
                    {},
                });

                if (error) return error;
                options->chafaCanvasMode = (uint32_t) value;
            }

            yyjson_val* colorSpace = yyjson_obj_get(val, "colorSpace");
            if (colorSpace)
            {
                int value;
                const char* error = ffJsonConfigParseEnum(colorSpace, &value, (FFKeyValuePair[]) {
                    { "RGB", 0 },
                    { "DIN99D", 1 },
                    {},
                });

                if (error) return error;
                options->chafaColorSpace = (uint32_t) value;
            }

            yyjson_val* ditherMode = yyjson_obj_get(val, "ditherMode");
            if (ditherMode)
            {
                int value;
                const char* error = ffJsonConfigParseEnum(ditherMode, &value, (FFKeyValuePair[]) {
                    { "NONE", 0 },
                    { "ORDERED", 1 },
                    { "DIFFUSION", 2 },
                    {},
                });

                if (error) return error;
                options->chafaDitherMode = (uint32_t) value;
            }
            continue;
        }
        else
            return "Unknown logo key";
    }

    return NULL;
}

void ffGenerateLogoJsonConfig(FFLogoOptions* options, yyjson_mut_doc* doc)
{
    __attribute__((__cleanup__(ffDestroyLogoOptions))) FFLogoOptions defaultOptions;
    ffInitLogoOptions(&defaultOptions);

    yyjson_mut_val* obj = yyjson_mut_obj(doc);

    if (options->type != defaultOptions.type)
    {
        switch (options->type)
        {
            case FF_LOGO_TYPE_NONE:
                yyjson_mut_obj_add_null(doc, doc->root, "logo");
                return;
            case FF_LOGO_TYPE_BUILTIN:
                yyjson_mut_obj_add_str(doc, obj, "type", "builtin");
                break;
            case FF_LOGO_TYPE_SMALL:
                yyjson_mut_obj_add_str(doc, obj, "type", "small");
                break;
            case FF_LOGO_TYPE_FILE:
                yyjson_mut_obj_add_str(doc, obj, "type", "file");
                break;
            case FF_LOGO_TYPE_FILE_RAW:
                yyjson_mut_obj_add_str(doc, obj, "type", "file-raw");
                break;
            case FF_LOGO_TYPE_DATA:
                yyjson_mut_obj_add_str(doc, obj, "type", "data");
                break;
            case FF_LOGO_TYPE_DATA_RAW:
                yyjson_mut_obj_add_str(doc, obj, "type", "data-raw");
                break;
            case FF_LOGO_TYPE_IMAGE_SIXEL:
                yyjson_mut_obj_add_str(doc, obj, "type", "sixel");
                break;
            case FF_LOGO_TYPE_IMAGE_KITTY:
                yyjson_mut_obj_add_str(doc, obj, "type", "kitty");
                break;
            case FF_LOGO_TYPE_IMAGE_ITERM:
                yyjson_mut_obj_add_str(doc, obj, "type", "iterm");
                break;
            case FF_LOGO_TYPE_IMAGE_CHAFA:
                yyjson_mut_obj_add_str(doc, obj, "type", "chafa");
                break;
            case FF_LOGO_TYPE_IMAGE_RAW:
                yyjson_mut_obj_add_str(doc, obj, "type", "raw");
                break;
            default:
                yyjson_mut_obj_add_str(doc, obj, "type", "auto");
                break;
        }
    }

    if (!ffStrbufEqual(&options->source, &defaultOptions.source))
        yyjson_mut_obj_add_str(doc, obj, "source", options->source.chars);

    {
        yyjson_mut_val* color = yyjson_mut_arr(doc);
        for (int i = 0; i < FASTFETCH_LOGO_MAX_COLORS; i++)
        {
            if (!ffStrbufEqual(&options->colors[i], &defaultOptions.colors[i]))
                yyjson_mut_arr_add_strbuf(doc, color, &options->colors[i]);
        }
        if (yyjson_mut_arr_size(color) > 0)
            yyjson_mut_obj_add_val(doc, obj, "color", color);
    }

    if (options->width != defaultOptions.width)
        yyjson_mut_obj_add_uint(doc, obj, "width", options->width);

    if (options->height != defaultOptions.height)
        yyjson_mut_obj_add_uint(doc, obj, "height", options->height);

    {
        yyjson_mut_val* padding = yyjson_mut_obj(doc);
        if (options->paddingTop != defaultOptions.paddingTop)
            yyjson_mut_obj_add_uint(doc, padding, "top", options->paddingTop);
        if (options->paddingLeft != defaultOptions.paddingLeft)
            yyjson_mut_obj_add_uint(doc, padding, "left", options->paddingLeft);
        if (options->paddingRight != defaultOptions.paddingRight)
            yyjson_mut_obj_add_uint(doc, padding, "right", options->paddingRight);

        if (yyjson_mut_obj_size(padding) > 0)
            yyjson_mut_obj_add_val(doc, obj, "padding", padding);
    }

    if (options->printRemaining != defaultOptions.printRemaining)
        yyjson_mut_obj_add_bool(doc, obj, "printRemaining", options->printRemaining);

    if (options->preserveAspectRadio != defaultOptions.preserveAspectRadio)
        yyjson_mut_obj_add_bool(doc, obj, "preserveAspectRadio", options->preserveAspectRadio);

    if (options->recache != defaultOptions.recache)
        yyjson_mut_obj_add_bool(doc, obj, "recache", options->recache);

    if (options->separate != defaultOptions.separate)
        yyjson_mut_obj_add_bool(doc, obj, "separate", options->separate);

    {
        yyjson_mut_val* chafa = yyjson_mut_obj(doc);
        if (options->chafaFgOnly != defaultOptions.chafaFgOnly)
            yyjson_mut_obj_add_bool(doc, chafa, "fgOnly", options->chafaFgOnly);
        if (!ffStrbufEqual(&options->chafaSymbols, &defaultOptions.chafaSymbols))
            yyjson_mut_obj_add_strbuf(doc, chafa, "symbols", &options->chafaSymbols);
        if (options->chafaCanvasMode != defaultOptions.chafaCanvasMode && options->chafaCanvasMode <= 7)
        {
            yyjson_mut_obj_add_str(doc, chafa, "canvasMode", ((const char* []) {
                "TRUECOLOR",
                "INDEXED_256",
                "INDEXED_240",
                "INDEXED_16",
                "FGBG_BGFG",
                "FGBG",
                "INDEXED_8",
                "INDEXED_16_8",
            })[options->chafaCanvasMode]);
        }
        if (options->chafaColorSpace != defaultOptions.chafaColorSpace && options->chafaColorSpace <= 1)
        {
            yyjson_mut_obj_add_str(doc, chafa, "colorSpace", ((const char* []) {
                "RGB",
                "DIN99D",
            })[options->chafaColorSpace]);
        }
        if (options->chafaDitherMode != defaultOptions.chafaDitherMode && options->chafaDitherMode <= 2)
        {
            yyjson_mut_obj_add_str(doc, chafa, "ditherMode", ((const char* []) {
                "NONE",
                "ORDERED",
                "DIFFUSION",
            })[options->chafaDitherMode]);
        }

        if (yyjson_mut_obj_size(chafa) > 0)
            yyjson_mut_obj_add_val(doc, obj, "chafa", chafa);
    }

    if (yyjson_mut_obj_size(obj) > 0)
        yyjson_mut_obj_add_val(doc, doc->root, "logo", obj);
}

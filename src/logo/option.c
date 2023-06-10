#include "logo/logo.h"

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

    options->chafaFgOnly = false;
    ffStrbufInitS(&options->chafaSymbols, "block+border+space-wide-inverted"); // Chafa default
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
            ffOptionParseString(key, value, &options->source);

            //this is usally wanted when using the none logo
            if(strcasecmp(value, "none") == 0)
            {
                options->paddingTop = 0;
                options->paddingRight = 0;
                options->paddingLeft = 0;
                options->type = FF_LOGO_TYPE_NONE;
            }
        }
        else if(strcasecmp(subKey, "type") == 0)
        {
            options->type = (FFLogoType) ffOptionParseEnum(key, value, (FFKeyValuePair[]) {
                { "auto", FF_LOGO_TYPE_AUTO },
                { "builtin", FF_LOGO_TYPE_BUILTIN },
                { "file", FF_LOGO_TYPE_FILE },
                { "file-raw", FF_LOGO_TYPE_FILE_RAW },
                { "data", FF_LOGO_TYPE_DATA },
                { "data-raw", FF_LOGO_TYPE_DATA_RAW },
                { "sixel", FF_LOGO_TYPE_IMAGE_SIXEL },
                { "kitty", FF_LOGO_TYPE_IMAGE_KITTY },
                { "iterm", FF_LOGO_TYPE_IMAGE_ITERM },
                { "chafa", FF_LOGO_TYPE_IMAGE_CHAFA },
                { "raw", FF_LOGO_TYPE_IMAGE_RAW },
                { "none", FF_LOGO_TYPE_NONE },
                {},
            });
        }
        else if(strncasecmp(subKey, "color-", strlen("color-")) && key[13] != '\0' && key[14] == '\0') // matches "--logo-color-*"
        {
            //Map the number to an array index, so that '1' -> 0, '2' -> 1, etc.
            int index = (int)key[13] - 49;

            //Match only --logo-color-[1-9]
            if(index < 0 || index >= FASTFETCH_LOGO_MAX_COLORS)
            {
                fprintf(stderr, "Error: invalid --color-[1-9] index: %c\n", key[13]);
                exit(472);
            }

            ffOptionParseColor(key, value, &options->colors[index]);
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
        else if(strcasecmp(key, "raw") == 0)
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

#pragma once

#include "util/FFstrbuf.h"

#define FASTFETCH_LOGO_MAX_COLORS 9 //two digits would make parsing much more complicated (index 1 - 9)

typedef enum FFLogoType
{
    FF_LOGO_TYPE_AUTO,        //if something is given, first try builtin, then file. Otherwise detect logo
    FF_LOGO_TYPE_BUILTIN,     //builtin ascii art
    FF_LOGO_TYPE_SMALL,       //builtin ascii art, small version
    FF_LOGO_TYPE_FILE,        //text file, printed with color code replacement
    FF_LOGO_TYPE_FILE_RAW,    //text file, printed as is
    FF_LOGO_TYPE_DATA,        //text data, printed with color code replacement
    FF_LOGO_TYPE_DATA_RAW,    //text data, printed as is
    FF_LOGO_TYPE_IMAGE_SIXEL, //image file, printed as sixel codes.
    FF_LOGO_TYPE_IMAGE_KITTY, //image file, printed as kitty graphics protocol
    FF_LOGO_TYPE_IMAGE_ITERM, //image file, printed as iterm graphics protocol
    FF_LOGO_TYPE_IMAGE_CHAFA, //image file, printed as ascii art using libchafa
    FF_LOGO_TYPE_IMAGE_RAW,   //image file, printed as raw binary string
    FF_LOGO_TYPE_NONE,        //--logo none
} FFLogoType;

typedef struct FFLogoOptions
{
    FFstrbuf source;
    FFLogoType type;
    FFstrbuf colors[FASTFETCH_LOGO_MAX_COLORS];
    uint32_t width;
    uint32_t height;
    uint32_t paddingTop;
    uint32_t paddingLeft;
    uint32_t paddingRight;
    bool printRemaining;
    bool preserveAspectRadio;

    bool chafaFgOnly;
    FFstrbuf chafaSymbols;
    uint32_t chafaCanvasMode;
    uint32_t chafaColorSpace;
    uint32_t chafaDitherMode;
} FFLogoOptions;

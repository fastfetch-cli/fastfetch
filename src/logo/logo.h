#pragma once

#include "fastfetch.h"

typedef enum FFLogoLineType
{
    FF_LOGO_LINE_TYPE_NORMAL = 0,
    FF_LOGO_LINE_TYPE_SMALL_BIT = 1 << 0, // The names of small logo must end with `_small` or `-small`
    FF_LOGO_LINE_TYPE_ALTER_BIT = 1 << 1,
} FFLogoLineType;

typedef struct FFlogo
{
    const char* lines;
    const char* names[FASTFETCH_LOGO_MAX_NAMES];
    const char* colors[FASTFETCH_LOGO_MAX_COLORS];
    const char* colorKeys;
    const char* colorTitle;
    FFLogoLineType type;
} FFlogo;

//logo.c
void ffLogoPrintChars(const char* data, bool doColorReplacement);

//builtin.c
extern const FFlogo* ffLogoBuiltins[];
extern const FFlogo ffLogoUnknown;

//image/image.c
bool ffLogoPrintImageIfExists(FFLogoType type, bool printError);

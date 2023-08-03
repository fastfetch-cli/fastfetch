#pragma once

#ifndef FASTFETCH_INCLUDED_LOGO_logo
#define FASTFETCH_INCLUDED_LOGO_logo

#include "fastfetch.h"

typedef struct FFlogo
{
    const char* lines;
    const char* names[FASTFETCH_LOGO_MAX_NAMES];
    const char* colors[FASTFETCH_LOGO_MAX_COLORS];
    const char* colorKeys;
    const char* colorTitle;
    bool small; // The names of small logo must end with `_small` or `-small`
} FFlogo;

//logo.c
void ffLogoPrintChars(const char* data, bool doColorReplacement);

//builtin.c
extern const FFlogo ffLogoBuiltins[];
extern const uint32_t ffLogoBuiltinLength;

//image/image.c
bool ffLogoPrintImageIfExists(FFLogoType type, bool printError);

//option.c
void ffInitLogoOptions(FFLogoOptions* options);
bool ffParseLogoCommandOptions(FFLogoOptions* options, const char* key, const char* value);
void ffDestroyLogoOptions(FFLogoOptions* options);
const char* ffParseLogoJsonConfig();

#endif

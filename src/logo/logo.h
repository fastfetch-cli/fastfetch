#pragma once

#ifndef FASTFETCH_INCLUDED_LOGO_logo
#define FASTFETCH_INCLUDED_LOGO_logo

#include "fastfetch.h"

typedef struct FFlogo
{
    const char* data;
    const char** names; //Null terminated
    const char** builtinColors; //Null terminated
    const char* colorKeys;
    const char* colorTitle;
} FFlogo;

typedef const FFlogo*(*GetLogoMethod)();

//logo.c
void ffLogoPrintChars(const char* data, bool doColorReplacement);

//builtin.c
const FFlogo* ffLogoBuiltinGetUnknown();
GetLogoMethod* ffLogoBuiltinGetAll();

//image/image.c
bool ffLogoPrintImageIfExists(FFLogoType type, bool printError);

//option.c
void ffInitLogoOptions(FFLogoOptions* options);
bool ffParseLogoCommandOptions(FFLogoOptions* options, const char* key, const char* value);
void ffDestroyLogoOptions(FFLogoOptions* options);
const char* ffParseLogoJsonConfig();

#endif

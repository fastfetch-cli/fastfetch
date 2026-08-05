#pragma once

#include "fastfetch.h"

typedef enum FFLogoLineType: uint8_t {
    FF_LOGO_LINE_TYPE_NORMAL = 0,
    FF_LOGO_LINE_TYPE_SMALL_BIT = 1 << 0, // The names of small logo must end with `_small` or `-small`
    FF_LOGO_LINE_TYPE_ALTER_BIT = 1 << 1,
} FFLogoLineType;

typedef enum FFLogoSize: uint8_t {
    FF_LOGO_SIZE_UNKNOWN,
    FF_LOGO_SIZE_NORMAL,
    FF_LOGO_SIZE_SMALL,
} FFLogoSize;

typedef struct FFlogo {
    const char* lines;
    const char* names[FASTFETCH_LOGO_MAX_NAMES];
    const char* colors[FASTFETCH_LOGO_MAX_COLORS];
    const char* colorKeys;
    const char* colorTitle;
    FFLogoLineType type;
} FFlogo;

// logo.c
void ffLogoPrint(void);
void ffLogoPrintChars(const char* data, bool doColorReplacement);
void ffLogoPrintLine(void);
void ffLogoPrintRemaining(void);
void ffLogoBuiltinPrint(void);
void ffLogoBuiltinList(void);
void ffLogoBuiltinListAutocompletion(void);
void ffLogoPrintDetected(FFLogoSize size);
const FFlogo* ffLogoGetBuiltinForName(const FFstrbuf* name, FFLogoSize size);
const FFlogo* ffLogoGetBuiltinDetected(FFLogoSize size);

// builtin.c
extern const FFlogo* ffLogoBuiltins[];
extern const FFlogo ffLogoUnknown;

// image/image.c
bool ffLogoPrintImageIfExists(FFLogoType type, bool printError);

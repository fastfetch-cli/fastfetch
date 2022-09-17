#pragma once

#ifndef FF_INCLUDED_common_font
#define FF_INCLUDED_common_font

#include "util/FFstrbuf.h"
#include "util/FFlist.h"

typedef struct FFfont
{
    FFstrbuf pretty;
    FFstrbuf name;
    FFstrbuf size;
    FFlist styles;
} FFfont;

void ffFontInitQt(FFfont* font, const char* data);
void ffFontInitPango(FFfont* font, const char* data);
void ffFontInitCopy(FFfont* font, const char* name);
void ffFontInitValues(FFfont* font, const char* name, const char* size);
void ffFontInitWithSpace(FFfont* font, const char* rawName);
void ffFontDestroy(FFfont* font);

#endif

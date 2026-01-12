#pragma once

#include "common/FFstrbuf.h"
#include "common/FFlist.h"

typedef struct FFfont
{
    FFstrbuf pretty;
    FFstrbuf name;
    FFstrbuf size;
    FFlist styles;
} FFfont;

void ffFontInit(FFfont* font);
void ffFontInitQt(FFfont* font, const char* data);
void ffFontInitPango(FFfont* font, const char* data);
void ffFontInitValues(FFfont* font, const char* name, const char* size);
void ffFontInitXlfd(FFfont* font, const char* xlfd);
void ffFontInitXft(FFfont* font, const char* xft);
void ffFontInitMoveValues(FFfont* font, FFstrbuf* name, FFstrbuf* size, FFstrbuf* style);
void ffFontInitWithSpace(FFfont* font, const char* rawName);
void ffFontDestroy(FFfont* font);

static inline void ffFontInitCopy(FFfont* font, const char* name)
{
    ffFontInitValues(font, name, NULL);
}

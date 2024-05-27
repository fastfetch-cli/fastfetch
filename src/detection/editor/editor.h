#pragma once

#include "fastfetch.h"

typedef struct FFEditorResult
{
    const char* type;
    FFstrbuf name;
    FFstrbuf exe;
    FFstrbuf path;
    FFstrbuf version;
} FFEditorResult;

const char* ffDetectEditor(FFEditorResult* result);

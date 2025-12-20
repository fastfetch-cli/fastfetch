#pragma once

#include "fastfetch.h"
#include "modules/editor/option.h"

typedef struct FFEditorResult
{
    const char* type;
    FFstrbuf name;
    FFstrbuf exe;
    FFstrbuf path;
    FFstrbuf version;
} FFEditorResult;

const char* ffDetectEditor(FFEditorResult* result);

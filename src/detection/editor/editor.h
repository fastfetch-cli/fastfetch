#pragma once

#include "fastfetch.h"

typedef struct FFEditorResult
{
    FFstrbuf name;
    FFstrbuf exe;
    FFstrbuf path;
    FFstrbuf version;
} FFEditorResult;

const char* ffDetectEditor(FFEditorResult* result);

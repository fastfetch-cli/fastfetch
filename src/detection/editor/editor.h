#pragma once

#include "fastfetch.h"

typedef struct FFEditorResult
{
    FFstrbuf name;
    const char* exe;
    FFstrbuf path;
    FFstrbuf version;
} FFEditorResult;

const char* ffDetectEditor(FFEditorResult* result);

#pragma once

#include "fastfetch.h"

typedef struct FFOSResult
{
    FFstrbuf name;
    FFstrbuf prettyName;
    FFstrbuf id;
    FFstrbuf idLike;
    FFstrbuf variant;
    FFstrbuf variantID;
    FFstrbuf version;
    FFstrbuf versionID;
    FFstrbuf codename;
    FFstrbuf buildID;
} FFOSResult;

const FFOSResult* ffDetectOS();

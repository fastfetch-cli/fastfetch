#pragma once

#include "fastfetch.h"

typedef struct FFVersionResult
{
    const char* projectName;
    const char* architecture;
    const char* version;
    const char* versionTweak;
    const char* cmakeBuiltType;
    const char* compileTime;
    const char* compiler;
    bool debugMode;
} FFVersionResult;

void ffDetectVersion(FFVersionResult* version);

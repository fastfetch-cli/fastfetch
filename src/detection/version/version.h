#pragma once

#ifndef FF_INCLUDED_detection_version_version
#define FF_INCLUDED_detection_version_version

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

#endif

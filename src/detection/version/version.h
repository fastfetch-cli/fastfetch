#pragma once

#include "fastfetch.h"

typedef struct FFVersionResult
{
    const char* projectName;
    const char* sysName;
    const char* architecture;
    const char* version;
    const char* versionTweak;
    const char* versionGit;
    const char* cmakeBuiltType;
    const char* compileTime;
    const char* compiler;
    bool debugMode;
} FFVersionResult;

extern FFVersionResult ffVersionResult;

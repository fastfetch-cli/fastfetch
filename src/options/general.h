#pragma once

#include "util/FFstrbuf.h"

typedef struct FFGeneralOptions
{
    bool allowSlowOperations;
    bool multithreading;
    int32_t processingTimeout;

    // Module options that cannot be put in module option structure
    #if defined(__linux__) || defined(__FreeBSD__)
    FFstrbuf playerName;
    FFstrbuf osFile;
    bool escapeBedrock;
    bool dsForceDrm;
    #elif defined(_WIN32)
    int32_t wmiTimeout;
    #endif
} FFGeneralOptions;

const char* ffParseGeneralJsonConfig(FFGeneralOptions* options, yyjson_val* root);
bool ffParseGeneralCommandOptions(FFGeneralOptions* options, const char* key, const char* value);
void ffInitGeneralOptions(FFGeneralOptions* options);
void ffDestroyGeneralOptions(FFGeneralOptions* options);

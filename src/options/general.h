#pragma once

#include "util/FFstrbuf.h"

typedef struct FFOptionsGeneral
{
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
} FFOptionsGeneral;

const char* ffOptionsParseGeneralJsonConfig(FFOptionsGeneral* options, yyjson_val* root);
bool ffOptionsParseGeneralCommandLine(FFOptionsGeneral* options, const char* key, const char* value);
void ffOptionsInitGeneral(FFOptionsGeneral* options);
void ffOptionsDestroyGeneral(FFOptionsGeneral* options);
void ffOptionsGenerateGeneralJsonConfig(FFOptionsGeneral* options, yyjson_mut_doc* doc);

#pragma once

#include "util/FFstrbuf.h"

typedef enum FFDsForceDrmType
{
    FF_DS_FORCE_DRM_TYPE_FALSE = 0,   // Disable
    FF_DS_FORCE_DRM_TYPE_TRUE = 1,    // Try `libdrm`, then `sysfs` if libdrm failed
    FF_DS_FORCE_DRM_TYPE_SYSFS_ONLY, // Use `/sys/class/drm` only
} FFDsForceDrmType;

typedef struct FFOptionsGeneral
{
    bool multithreading;
    int32_t processingTimeout;
    bool detectVersion;

    // Module options that cannot be put in module option structure
    #if defined(__linux__) || defined(__FreeBSD__) || defined(__sun) || defined(__OpenBSD__)
    FFstrbuf playerName;
    bool escapeBedrock;
    FFDsForceDrmType dsForceDrm;
    #elif defined(_WIN32)
    int32_t wmiTimeout;
    #endif
} FFOptionsGeneral;

const char* ffOptionsParseGeneralJsonConfig(FFOptionsGeneral* options, yyjson_val* root);
bool ffOptionsParseGeneralCommandLine(FFOptionsGeneral* options, const char* key, const char* value);
void ffOptionsInitGeneral(FFOptionsGeneral* options);
void ffOptionsDestroyGeneral(FFOptionsGeneral* options);
void ffOptionsGenerateGeneralJsonConfig(FFOptionsGeneral* options, yyjson_mut_doc* doc);

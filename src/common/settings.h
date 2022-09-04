#pragma once

#ifndef FF_INCLUDED_common_settings
#define FF_INCLUDED_common_settings

#include "fastfetch.h"

typedef enum FFvarianttype
{
    FF_VARIANT_TYPE_STRING,
    FF_VARIANT_TYPE_BOOL,
    FF_VARIANT_TYPE_INT
} FFvarianttype;

typedef union FFvariant
{
    const char* strValue;
    int32_t intValue;
    struct
    {
        bool boolValueSet;
        bool boolValue;
    };
} FFvariant;

#define FF_VARIANT_NULL ((FFvariant){.strValue = NULL})

FFvariant ffSettingsGetDConf(FFinstance* instance, const char* key, FFvarianttype type);
FFvariant ffSettingsGetGSettings(FFinstance* instance, const char* schemaName, const char* path, const char* key, FFvarianttype type);
FFvariant ffSettingsGet(FFinstance* instance, const char* dconfKey, const char* gsettingsSchemaName, const char* gsettingsPath, const char* gsettingsKey, FFvarianttype type);
FFvariant ffSettingsGetXFConf(FFinstance* instance, const char* channelName, const char* propertyName, FFvarianttype type);

int ffSettingsGetSQLite3Int(const FFinstance* instance, const char* dbPath, const char* query);

#ifdef __ANDROID__
void ffSettingsGetAndroidProperty(const char* propName, FFstrbuf* result);
#endif

#endif

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

FFvariant ffSettingsGetDConf(const char* key, FFvarianttype type);
FFvariant ffSettingsGetGSettings(const char* schemaName, const char* path, const char* key, FFvarianttype type);
FFvariant ffSettingsGet(const char* dconfKey, const char* gsettingsSchemaName, const char* gsettingsPath, const char* gsettingsKey, FFvarianttype type);
FFvariant ffSettingsGetXFConf(const char* channelName, const char* propertyName, FFvarianttype type);

int ffSettingsGetSQLite3Int(const char* dbPath, const char* query);
bool ffSettingsGetSQLite3String(const char* dbPath, const char* query, FFstrbuf* result);

#ifdef __ANDROID__
bool ffSettingsGetAndroidProperty(const char* propName, FFstrbuf* result);
#elif defined(__FreeBSD__)
bool ffSettingsGetFreeBSDKenv(const char* propName, FFstrbuf* result);
#endif

#endif

#include "fastfetch.h"
#include "common/settings.h"
#include "common/library.h"
#include "common/io.h"

#include <pthread.h>
#include <string.h>

typedef enum FFInitState
{
    FF_INITSTATE_UNINITIALIZED = 0,
    FF_INITSTATE_SUCCESSFUL = 1,
    FF_INITSTATE_FAILED = 2
} FFInitState;

#define FF_LIBRARY_DATA_LOAD_INIT(dataObject, userLibraryName, ...) \
    static dataObject data; \
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; \
    static FFInitState initState = FF_INITSTATE_UNINITIALIZED; \
    pthread_mutex_lock(&mutex); \
    if(initState != FF_INITSTATE_UNINITIALIZED) {\
        pthread_mutex_unlock(&mutex); \
        return initState == FF_INITSTATE_SUCCESSFUL ? &data : NULL; \
    } \
    initState = FF_INITSTATE_SUCCESSFUL; \
    void* libraryHandle = ffLibraryLoad(&userLibraryName, __VA_ARGS__, NULL); \
    if(libraryHandle == NULL) { \
        initState = FF_INITSTATE_FAILED; \
        pthread_mutex_unlock(&mutex); \
        return NULL; \
    } \

#define FF_LIBRARY_DATA_LOAD_SYMBOL(symbolName) \
    data.ff ## symbolName = dlsym(libraryHandle, #symbolName); \
    if(data.ff ## symbolName == NULL) { \
        dlclose(libraryHandle); \
        initState = FF_INITSTATE_FAILED; \
        pthread_mutex_unlock(&mutex); \
        return NULL; \
    }

#define FF_LIBRARY_DATA_LOAD_RETURN \
    initState = FF_INITSTATE_SUCCESSFUL; \
    pthread_mutex_unlock(&mutex); \
    return &data;

#define FF_LIBRARY_DATA_LOAD_ERROR \
    { \
        dlclose(libraryHandle); \
        initState = FF_INITSTATE_FAILED; \
        pthread_mutex_unlock(&mutex); \
        return NULL; \
    }

#ifdef FF_HAVE_GIO
#include <gio/gio.h>

typedef struct GVariantGetters
{
    FF_LIBRARY_SYMBOL(g_variant_get_string)
    FF_LIBRARY_SYMBOL(g_variant_get_boolean)
    FF_LIBRARY_SYMBOL(g_variant_get_int32)
} GVariantGetters;

static FFvariant getGVariantValue(GVariant* variant, FFvarianttype type, const GVariantGetters* variantGetters)
{
    if(variant == NULL)
        return FF_VARIANT_NULL;
    else if(type == FF_VARIANT_TYPE_STRING)
        return (FFvariant) {.strValue = variantGetters->ffg_variant_get_string(variant, NULL)};
    else if(type == FF_VARIANT_TYPE_BOOL)
        return (FFvariant) {.boolValue = (bool) variantGetters->ffg_variant_get_boolean(variant), .boolValueSet = true};
    else if(type == FF_VARIANT_TYPE_INT)
        return (FFvariant) {.intValue = variantGetters->ffg_variant_get_int32(variant)};
    else
        return FF_VARIANT_NULL;
}

typedef struct GSettingsData
{
    FF_LIBRARY_SYMBOL(g_settings_schema_source_lookup)
    FF_LIBRARY_SYMBOL(g_settings_schema_has_key)
    FF_LIBRARY_SYMBOL(g_settings_new_full)
    FF_LIBRARY_SYMBOL(g_settings_get_value)
    FF_LIBRARY_SYMBOL(g_settings_get_user_value)
    FF_LIBRARY_SYMBOL(g_settings_get_default_value)
    FF_LIBRARY_SYMBOL(g_settings_schema_source_get_default)
    GSettingsSchemaSource* schemaSource;
    GVariantGetters variantGetters;
} GSettingsData;

static const GSettingsData* getGSettingsData(const FFinstance* instance)
{
    FF_LIBRARY_DATA_LOAD_INIT(GSettingsData, instance->config.libGIO, "libgio-2.0.so", 1);

    FF_LIBRARY_DATA_LOAD_SYMBOL(g_settings_schema_source_lookup)
    FF_LIBRARY_DATA_LOAD_SYMBOL(g_settings_schema_has_key)
    FF_LIBRARY_DATA_LOAD_SYMBOL(g_settings_new_full)
    FF_LIBRARY_DATA_LOAD_SYMBOL(g_settings_get_value)
    FF_LIBRARY_DATA_LOAD_SYMBOL(g_settings_get_user_value)
    FF_LIBRARY_DATA_LOAD_SYMBOL(g_settings_get_default_value)
    FF_LIBRARY_DATA_LOAD_SYMBOL(g_settings_schema_source_get_default)

    #define data data.variantGetters
    FF_LIBRARY_DATA_LOAD_SYMBOL(g_variant_get_string)
    FF_LIBRARY_DATA_LOAD_SYMBOL(g_variant_get_boolean)
    FF_LIBRARY_DATA_LOAD_SYMBOL(g_variant_get_int32)
    #undef data

    data.schemaSource = data.ffg_settings_schema_source_get_default();
    if(data.schemaSource == NULL)
        FF_LIBRARY_DATA_LOAD_ERROR

    FF_LIBRARY_DATA_LOAD_RETURN
}

FFvariant ffSettingsGetGSettings(const FFinstance* instance, const char* schemaName, const char* path, const char* key, FFvarianttype type)
{
    const GSettingsData* data = getGSettingsData(instance);
    if(data == NULL)
        return FF_VARIANT_NULL;

    GSettingsSchema* schema = data->ffg_settings_schema_source_lookup(data->schemaSource, schemaName, true);
    if(schema == NULL)
        return FF_VARIANT_NULL;

    if(data->ffg_settings_schema_has_key(schema, key) == false)
        return FF_VARIANT_NULL;

    GSettings* settings = data->ffg_settings_new_full(schema, NULL, path);
    if(settings == NULL)
        return FF_VARIANT_NULL;

    GVariant* variant = data->ffg_settings_get_value(settings, key);
    if(variant != NULL)
        return getGVariantValue(variant, type, &data->variantGetters);

    variant = data->ffg_settings_get_user_value(settings, key);
    if(variant != NULL)
        return getGVariantValue(variant, type, &data->variantGetters);

    variant = data->ffg_settings_get_default_value(settings, key);
    return getGVariantValue(variant, type, &data->variantGetters);
}
#else //FF_HAVE_GIO
FFvariant ffSettingsGetGSettings(const FFinstance* instance, const char* schemaName, const char* path, const char* key, FFvarianttype type)
{
    FF_UNUSED(instance, schemaName, path, key, type)
    return FF_VARIANT_NULL;
}
#endif //FF_HAVE_GIO

#ifdef FF_HAVE_DCONF
#include <dconf.h>

typedef struct DConfData
{
    FF_LIBRARY_SYMBOL(dconf_client_read_full)
    FF_LIBRARY_SYMBOL(dconf_client_new)
    GVariantGetters variantGetters;
    DConfClient* client;
} DConfData;

static const DConfData* getDConfData(const FFinstance* instance)
{
    FF_LIBRARY_DATA_LOAD_INIT(DConfData, instance->config.libDConf, "libdconf.so", 2);

    FF_LIBRARY_DATA_LOAD_SYMBOL(dconf_client_read_full)
    FF_LIBRARY_DATA_LOAD_SYMBOL(dconf_client_new)

    #define data data.variantGetters
    FF_LIBRARY_DATA_LOAD_SYMBOL(g_variant_get_string)
    FF_LIBRARY_DATA_LOAD_SYMBOL(g_variant_get_boolean)
    FF_LIBRARY_DATA_LOAD_SYMBOL(g_variant_get_int32)
    #undef data

    data.client = data.ffdconf_client_new();
    if(data.client == NULL)
        FF_LIBRARY_DATA_LOAD_ERROR

    FF_LIBRARY_DATA_LOAD_RETURN
}

FFvariant ffSettingsGetDConf(const FFinstance* instance, const char* key, FFvarianttype type)
{
    const DConfData* data = getDConfData(instance);
    if(data == NULL)
        return FF_VARIANT_NULL;

    GVariant* variant = data->ffdconf_client_read_full(data->client, key, DCONF_READ_FLAGS_NONE, NULL);
    if(variant != NULL)
        return getGVariantValue(variant, type, &data->variantGetters);

    variant = data->ffdconf_client_read_full(data->client, key, DCONF_READ_USER_VALUE, NULL);
    if(variant != NULL)
        return getGVariantValue(variant, type, &data->variantGetters);

    variant = data->ffdconf_client_read_full(data->client, key, DCONF_READ_DEFAULT_VALUE, NULL);
    return getGVariantValue(variant, type, &data->variantGetters);
}
#else //FF_HAVE_DCONF
FFvariant ffSettingsGetDConf(const FFinstance* instance, const char* key, FFvarianttype type)
{
    FF_UNUSED(instance, key, type)
    return FF_VARIANT_NULL;
}
#endif //FF_HAVE_DCONF

FFvariant ffSettingsGet(const FFinstance* instance, const char* dconfKey, const char* gsettingsSchemaName, const char* gsettingsPath, const char* gsettingsKey, FFvarianttype type)
{
    FFvariant gsettings = ffSettingsGetGSettings(instance, gsettingsSchemaName, gsettingsPath, gsettingsKey, type);

    if(
        (type == FF_VARIANT_TYPE_BOOL && gsettings.boolValueSet) ||
        (type != FF_VARIANT_TYPE_BOOL && gsettings.strValue != NULL)
    ) return gsettings;

    return ffSettingsGetDConf(instance, dconfKey, type);
}

#ifdef FF_HAVE_XFCONF
#include <xfconf/xfconf.h>

typedef struct XFConfData
{
    FF_LIBRARY_SYMBOL(xfconf_channel_get)
    FF_LIBRARY_SYMBOL(xfconf_channel_has_property)
    FF_LIBRARY_SYMBOL(xfconf_channel_get_string)
    FF_LIBRARY_SYMBOL(xfconf_channel_get_bool)
    FF_LIBRARY_SYMBOL(xfconf_channel_get_int)
    FF_LIBRARY_SYMBOL(xfconf_init)
} XFConfData;

static const XFConfData* getXFConfData(const FFinstance* instance)
{
    FF_LIBRARY_DATA_LOAD_INIT(XFConfData, instance->config.libXFConf, "libxfconf-0.so", 4);

    FF_LIBRARY_DATA_LOAD_SYMBOL(xfconf_channel_get)
    FF_LIBRARY_DATA_LOAD_SYMBOL(xfconf_channel_has_property)
    FF_LIBRARY_DATA_LOAD_SYMBOL(xfconf_channel_get_string)
    FF_LIBRARY_DATA_LOAD_SYMBOL(xfconf_channel_get_bool)
    FF_LIBRARY_DATA_LOAD_SYMBOL(xfconf_channel_get_int)
    FF_LIBRARY_DATA_LOAD_SYMBOL(xfconf_init)

    if(data.ffxfconf_init(NULL) == false)
        FF_LIBRARY_DATA_LOAD_ERROR

    FF_LIBRARY_DATA_LOAD_RETURN
}

FFvariant ffSettingsGetXFConf(const FFinstance* instance, const char* channelName, const char* propertyName, FFvarianttype type)
{
    const XFConfData* data = getXFConfData(instance);
    if(data == NULL)
        return FF_VARIANT_NULL;

    XfconfChannel* channel = data->ffxfconf_channel_get(channelName); // Never fails according to documentation but rather returns an empty channel

    if(!data->ffxfconf_channel_has_property(channel, propertyName))
        return FF_VARIANT_NULL;

    if(type == FF_VARIANT_TYPE_INT)
        return (FFvariant) {.intValue = data->ffxfconf_channel_get_int(channel, propertyName, 0)};

    if(type == FF_VARIANT_TYPE_STRING)
        return (FFvariant) {.strValue = data->ffxfconf_channel_get_string(channel, propertyName, NULL)};

    if(type == FF_VARIANT_TYPE_BOOL)
        return (FFvariant) {.boolValue = data->ffxfconf_channel_get_bool(channel, propertyName, false), .boolValueSet = true};

    return FF_VARIANT_NULL;
}
#else //FF_HAVE_XFCONF
FFvariant ffSettingsGetXFConf(const FFinstance* instance, const char* channelName, const char* propertyName, FFvarianttype type)
{
    FF_UNUSED(instance, channelName, propertyName, type)
    return FF_VARIANT_NULL;
}
#endif //FF_HAVE_XFCONF

#ifdef FF_HAVE_SQLITE3
#include <sqlite3.h>
#include <sys/stat.h>

typedef struct SQLiteData
{
    FF_LIBRARY_SYMBOL(sqlite3_open_v2)
    FF_LIBRARY_SYMBOL(sqlite3_prepare_v2)
    FF_LIBRARY_SYMBOL(sqlite3_step)
    FF_LIBRARY_SYMBOL(sqlite3_data_count)
    FF_LIBRARY_SYMBOL(sqlite3_column_int)
    FF_LIBRARY_SYMBOL(sqlite3_finalize)
    FF_LIBRARY_SYMBOL(sqlite3_close)
} SQLiteData;

static const SQLiteData* getSQLiteData(const FFinstance* instance)
{
    FF_LIBRARY_DATA_LOAD_INIT(SQLiteData, instance->config.libSQLite3, "libsqlite3.so", 1);

    FF_LIBRARY_DATA_LOAD_SYMBOL(sqlite3_open_v2)
    FF_LIBRARY_DATA_LOAD_SYMBOL(sqlite3_prepare_v2)
    FF_LIBRARY_DATA_LOAD_SYMBOL(sqlite3_step)
    FF_LIBRARY_DATA_LOAD_SYMBOL(sqlite3_data_count)
    FF_LIBRARY_DATA_LOAD_SYMBOL(sqlite3_column_int)
    FF_LIBRARY_DATA_LOAD_SYMBOL(sqlite3_finalize)
    FF_LIBRARY_DATA_LOAD_SYMBOL(sqlite3_close)

    FF_LIBRARY_DATA_LOAD_RETURN
}

int ffSettingsGetSQLite3Int(const FFinstance* instance, const char* dbPath, const char* query)
{
    if(!ffFileExists(dbPath, S_IFREG))
        return 0;

    const SQLiteData* data = getSQLiteData(instance);
    if(data == NULL)
        return 0;

    sqlite3* db;
    if(data->ffsqlite3_open_v2(dbPath, &db, SQLITE_OPEN_READONLY, NULL) != SQLITE_OK)
        return 0;

    sqlite3_stmt* stmt;
    if(data->ffsqlite3_prepare_v2(db, query, (int) strlen(query), &stmt, NULL) != SQLITE_OK)
    {
        data->ffsqlite3_close(db);
        return 0;
    }

    if(data->ffsqlite3_step(stmt) != SQLITE_ROW || data->ffsqlite3_data_count(stmt) < 1)
    {
        data->ffsqlite3_finalize(stmt);
        data->ffsqlite3_close(db);
        return 0;
    }

    int result = data->ffsqlite3_column_int(stmt, 0);

    data->ffsqlite3_finalize(stmt);
    data->ffsqlite3_close(db);

    return result;
}
#else //FF_HAVE_SQLITE3
int ffSettingsGetSQLite3Int(const FFinstance* instance, const char* dbPath, const char* query)
{
    FF_UNUSED(instance, dbPath, query)
    return 0;
}
#endif //FF_HAVE_SQLITE3

#ifdef __ANDROID__
#include <sys/system_properties.h>
void ffSettingsGetAndroidProperty(const char* propName, FFstrbuf* result) {
    ffStrbufEnsureFree(result, PROP_VALUE_MAX);
    result->length += (uint32_t) __system_property_get(propName, result->chars + result->length);
    result->chars[result->length] = '\0';
}
#endif //__ANDROID__

#ifdef __APPLE__
#include <sys/sysctl.h>
void ffSettingsGetAppleProperty(const char* propName, FFstrbuf* result)
{
    size_t neededLength;
    if(sysctlbyname(propName, NULL, &neededLength, NULL, 0) != 0 || neededLength == 0)
        return;

    ffStrbufEnsureFree(result, (uint32_t) neededLength);

    if(sysctlbyname(propName, result->chars + result->length, &neededLength, NULL, 0) == 0)
        result->length += (uint32_t) neededLength;

    result->chars[result->length] = '\0';
}

int ffSettingsGetAppleInt(const char* propName, int defaultValue)
{
    int result;
    size_t neededLength = sizeof(result);
    if(sysctlbyname(propName, &result, &neededLength, NULL, 0) != 0)
        return defaultValue;
    return result;
}

int64_t ffSettingsGetAppleInt64(const char* propName, int64_t defaultValue)
{
    int64_t result;
    size_t neededLength = sizeof(result);
    if(sysctlbyname(propName, &result, &neededLength, NULL, 0) != 0)
        return defaultValue;
    return result;
}
#endif //__APPLE__

#include "fastfetch.h"
#include "common/settings.h"
#include "common/library.h"
#include "common/thread.h"
#include "common/io/io.h"

#include <string.h>

typedef enum FFInitState
{
    FF_INITSTATE_UNINITIALIZED = 0,
    FF_INITSTATE_SUCCESSFUL = 1,
    FF_INITSTATE_FAILED = 2
} FFInitState;

#define FF_LIBRARY_DATA_LOAD_INIT(dataObject, userLibraryName, ...) \
    static dataObject data; \
    static FFThreadMutex mutex = FF_THREAD_MUTEX_INITIALIZER; \
    static FFInitState initState = FF_INITSTATE_UNINITIALIZED; \
    ffThreadMutexLock(&mutex); \
    if(initState != FF_INITSTATE_UNINITIALIZED) {\
        ffThreadMutexUnlock(&mutex); \
        return initState == FF_INITSTATE_SUCCESSFUL ? &data : NULL; \
    } \
    initState = FF_INITSTATE_SUCCESSFUL; \
    void* libraryHandle = ffLibraryLoad(&userLibraryName, __VA_ARGS__, NULL); \
    if(libraryHandle == NULL) { \
        initState = FF_INITSTATE_FAILED; \
        ffThreadMutexUnlock(&mutex); \
        return NULL; \
    } \

#define FF_LIBRARY_DATA_LOAD_SYMBOL(symbolName) \
    data.ff ## symbolName = dlsym(libraryHandle, #symbolName); \
    if(data.ff ## symbolName == NULL) { \
        dlclose(libraryHandle); \
        initState = FF_INITSTATE_FAILED; \
        ffThreadMutexUnlock(&mutex); \
        return NULL; \
    }

#define FF_LIBRARY_DATA_LOAD_RETURN \
    initState = FF_INITSTATE_SUCCESSFUL; \
    ffThreadMutexUnlock(&mutex); \
    return &data;

#define FF_LIBRARY_DATA_LOAD_ERROR \
    { \
        dlclose(libraryHandle); \
        initState = FF_INITSTATE_FAILED; \
        ffThreadMutexUnlock(&mutex); \
        return NULL; \
    }

#ifdef FF_HAVE_GIO
#include <gio/gio.h>

typedef struct GVariantGetters
{
    FF_LIBRARY_SYMBOL(g_variant_dup_string)
    FF_LIBRARY_SYMBOL(g_variant_get_boolean)
    FF_LIBRARY_SYMBOL(g_variant_get_int32)
    FF_LIBRARY_SYMBOL(g_variant_unref)
} GVariantGetters;

static FFvariant getGVariantValue(GVariant* variant, FFvarianttype type, const GVariantGetters* variantGetters)
{
    FFvariant result;

    if(variant == NULL)
        result = FF_VARIANT_NULL;
    else if(type == FF_VARIANT_TYPE_STRING)
        result = (FFvariant) {.strValue = variantGetters->ffg_variant_dup_string(variant, NULL)}; // Dup string, so that variant itself can be freed
    else if(type == FF_VARIANT_TYPE_BOOL)
        result = (FFvariant) {.boolValue = (bool) variantGetters->ffg_variant_get_boolean(variant), .boolValueSet = true};
    else if(type == FF_VARIANT_TYPE_INT)
        result = (FFvariant) {.intValue = variantGetters->ffg_variant_get_int32(variant)};
    else
        result = FF_VARIANT_NULL;

    if(variant)
        variantGetters->ffg_variant_unref(variant);

    return result;
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

static const GSettingsData* getGSettingsData(void)
{
    FF_LIBRARY_DATA_LOAD_INIT(GSettingsData, instance.config.libGIO, "libgio-2.0" FF_LIBRARY_EXTENSION, 1);

    FF_LIBRARY_DATA_LOAD_SYMBOL(g_settings_schema_source_lookup)
    FF_LIBRARY_DATA_LOAD_SYMBOL(g_settings_schema_has_key)
    FF_LIBRARY_DATA_LOAD_SYMBOL(g_settings_new_full)
    FF_LIBRARY_DATA_LOAD_SYMBOL(g_settings_get_value)
    FF_LIBRARY_DATA_LOAD_SYMBOL(g_settings_get_user_value)
    FF_LIBRARY_DATA_LOAD_SYMBOL(g_settings_get_default_value)
    FF_LIBRARY_DATA_LOAD_SYMBOL(g_settings_schema_source_get_default)

    #define data data.variantGetters
    FF_LIBRARY_DATA_LOAD_SYMBOL(g_variant_dup_string)
    FF_LIBRARY_DATA_LOAD_SYMBOL(g_variant_get_boolean)
    FF_LIBRARY_DATA_LOAD_SYMBOL(g_variant_get_int32)
    FF_LIBRARY_DATA_LOAD_SYMBOL(g_variant_unref);
    #undef data

    data.schemaSource = data.ffg_settings_schema_source_get_default();
    if(data.schemaSource == NULL)
        FF_LIBRARY_DATA_LOAD_ERROR

    FF_LIBRARY_DATA_LOAD_RETURN
}

FFvariant ffSettingsGetGSettings(const char* schemaName, const char* path, const char* key, FFvarianttype type)
{
    const GSettingsData* data = getGSettingsData();
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
FFvariant ffSettingsGetGSettings(const char* schemaName, const char* path, const char* key, FFvarianttype type)
{
    FF_UNUSED(schemaName, path, key, type)
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

static const DConfData* getDConfData(void)
{
    FF_LIBRARY_DATA_LOAD_INIT(DConfData, instance.config.libDConf, "libdconf" FF_LIBRARY_EXTENSION, 2);

    FF_LIBRARY_DATA_LOAD_SYMBOL(dconf_client_read_full)
    FF_LIBRARY_DATA_LOAD_SYMBOL(dconf_client_new)

    #define data data.variantGetters
    FF_LIBRARY_DATA_LOAD_SYMBOL(g_variant_dup_string)
    FF_LIBRARY_DATA_LOAD_SYMBOL(g_variant_get_boolean)
    FF_LIBRARY_DATA_LOAD_SYMBOL(g_variant_get_int32)
    FF_LIBRARY_DATA_LOAD_SYMBOL(g_variant_unref)
    #undef data

    data.client = data.ffdconf_client_new();
    if(data.client == NULL)
        FF_LIBRARY_DATA_LOAD_ERROR

    FF_LIBRARY_DATA_LOAD_RETURN
}

FFvariant ffSettingsGetDConf(const char* key, FFvarianttype type)
{
    const DConfData* data = getDConfData();
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
FFvariant ffSettingsGetDConf(const char* key, FFvarianttype type)
{
    FF_UNUSED(key, type)
    return FF_VARIANT_NULL;
}
#endif //FF_HAVE_DCONF

FFvariant ffSettingsGet(const char* dconfKey, const char* gsettingsSchemaName, const char* gsettingsPath, const char* gsettingsKey, FFvarianttype type)
{
    FFvariant gsettings = ffSettingsGetGSettings(gsettingsSchemaName, gsettingsPath, gsettingsKey, type);

    if(
        (type == FF_VARIANT_TYPE_BOOL && gsettings.boolValueSet) ||
        (type != FF_VARIANT_TYPE_BOOL && gsettings.strValue != NULL)
    ) return gsettings;

    return ffSettingsGetDConf(dconfKey, type);
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

static const XFConfData* getXFConfData(void)
{
    FF_LIBRARY_DATA_LOAD_INIT(XFConfData, instance.config.libXFConf, "libxfconf-0" FF_LIBRARY_EXTENSION, 4);

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

FFvariant ffSettingsGetXFConf(const char* channelName, const char* propertyName, FFvarianttype type)
{
    const XFConfData* data = getXFConfData();
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
FFvariant ffSettingsGetXFConf(const char* channelName, const char* propertyName, FFvarianttype type)
{
    FF_UNUSED(channelName, propertyName, type)
    return FF_VARIANT_NULL;
}
#endif //FF_HAVE_XFCONF

#ifdef FF_HAVE_SQLITE3
#include <sqlite3.h>

typedef struct SQLiteData
{
    FF_LIBRARY_SYMBOL(sqlite3_open_v2)
    FF_LIBRARY_SYMBOL(sqlite3_prepare_v2)
    FF_LIBRARY_SYMBOL(sqlite3_step)
    FF_LIBRARY_SYMBOL(sqlite3_data_count)
    FF_LIBRARY_SYMBOL(sqlite3_column_int)
    FF_LIBRARY_SYMBOL(sqlite3_column_text)
    FF_LIBRARY_SYMBOL(sqlite3_finalize)
    FF_LIBRARY_SYMBOL(sqlite3_close)
} SQLiteData;

static const SQLiteData* getSQLiteData(void)
{
    FF_LIBRARY_DATA_LOAD_INIT(SQLiteData, instance.config.libSQLite3, "libsqlite3" FF_LIBRARY_EXTENSION, 1);

    FF_LIBRARY_DATA_LOAD_SYMBOL(sqlite3_open_v2)
    FF_LIBRARY_DATA_LOAD_SYMBOL(sqlite3_prepare_v2)
    FF_LIBRARY_DATA_LOAD_SYMBOL(sqlite3_step)
    FF_LIBRARY_DATA_LOAD_SYMBOL(sqlite3_data_count)
    FF_LIBRARY_DATA_LOAD_SYMBOL(sqlite3_column_int)
    FF_LIBRARY_DATA_LOAD_SYMBOL(sqlite3_column_text)
    FF_LIBRARY_DATA_LOAD_SYMBOL(sqlite3_finalize)
    FF_LIBRARY_DATA_LOAD_SYMBOL(sqlite3_close)

    FF_LIBRARY_DATA_LOAD_RETURN
}

int ffSettingsGetSQLite3Int(const char* dbPath, const char* query)
{
    if(!ffPathExists(dbPath, FF_PATHTYPE_FILE))
        return 0;

    const SQLiteData* data = getSQLiteData();
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

bool ffSettingsGetSQLite3String(const char* dbPath, const char* query, FFstrbuf* result)
{
    if(!ffPathExists(dbPath, FF_PATHTYPE_FILE))
        return false;

    const SQLiteData* data = getSQLiteData();
    if(data == NULL)
        return false;

    sqlite3* db;
    if(data->ffsqlite3_open_v2(dbPath, &db, SQLITE_OPEN_READONLY, NULL) != SQLITE_OK)
        return false;

    sqlite3_stmt* stmt;
    if(data->ffsqlite3_prepare_v2(db, query, (int) strlen(query), &stmt, NULL) != SQLITE_OK)
    {
        data->ffsqlite3_close(db);
        return false;
    }

    if(data->ffsqlite3_step(stmt) != SQLITE_ROW || data->ffsqlite3_data_count(stmt) < 1)
    {
        data->ffsqlite3_finalize(stmt);
        data->ffsqlite3_close(db);
        return false;
    }

    ffStrbufSetS(result, (const char *) data->ffsqlite3_column_text(stmt, 0));

    data->ffsqlite3_finalize(stmt);
    data->ffsqlite3_close(db);

    return true;
}
#else //FF_HAVE_SQLITE3
int ffSettingsGetSQLite3Int(const char* dbPath, const char* query)
{
    FF_UNUSED(dbPath, query)
    return 0;
}
bool ffSettingsGetSQLite3String(const char* dbPath, const char* query, FFstrbuf* result)
{
    FF_UNUSED(dbPath, query, result)
    return false;
}
#endif //FF_HAVE_SQLITE3

#ifdef __ANDROID__
#include <sys/system_properties.h>
bool ffSettingsGetAndroidProperty(const char* propName, FFstrbuf* result) {
    ffStrbufEnsureFree(result, PROP_VALUE_MAX);
    int len = __system_property_get(propName, result->chars + result->length);
    if (len <= 0) return false;
    result->length += (uint32_t) len;
    result->chars[result->length] = '\0';
    return true;
}
#elif defined(__FreeBSD__)
#include <kenv.h>
bool ffSettingsGetFreeBSDKenv(const char* propName, FFstrbuf* result)
{
    //https://wiki.ghostbsd.org/index.php/Kenv
    ffStrbufEnsureFree(result, KENV_MVALLEN);
    int len = kenv(KENV_GET, propName, result->chars + result->length, KENV_MVALLEN);
    if (len <= 0) return false;
    result->length += (uint32_t) len;
    result->chars[result->length] = '\0';
    return true;
}
#endif

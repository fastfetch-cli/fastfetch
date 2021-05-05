#include "fastfetch.h"

#include <dlfcn.h>
#include <pthread.h>
#include <gio/gio.h>
#include <dconf.h>

typedef struct DConfData
{
    GVariant*(*ffdconf_client_read_full)(DConfClient*, const gchar*, DConfReadFlags, const GQueue*);
    const gchar*(*ffg_variant_get_string)(GVariant*, gsize*);
    DConfClient* client;
} DConfData;

static const char* getDConfValue(DConfData* data, const char* key)
{
    if(data->client == NULL)
        return NULL;

    GVariant* variant = data->ffdconf_client_read_full(data->client, key, DCONF_READ_FLAGS_NONE, NULL);

    if(variant == NULL)
        variant = data->ffdconf_client_read_full(data->client, key, DCONF_READ_USER_VALUE, NULL);

    if(variant == NULL)
        variant = data->ffdconf_client_read_full(data->client, key, DCONF_READ_DEFAULT_VALUE, NULL);

    if(variant == NULL)
        return NULL;

    return data->ffg_variant_get_string(variant, NULL);
}

const char* ffSettingsGetDConf(FFinstance* instance, const char* key)
{
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    static bool init = false;

    static DConfData data;

    pthread_mutex_lock(&mutex);

    if(init)
    {
        pthread_mutex_unlock(&mutex);
        return getDConfValue(&data, key);
    }

    init = true;

    data.client = NULL; //error indicator

    void* library = dlopen(instance->config.libDConf.length == 0 ? "libdconf.so" : instance->config.libDConf.chars, RTLD_LAZY);
    if(library == NULL)
    {
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    data.ffdconf_client_read_full = dlsym(library, "dconf_client_read_full");
    data.ffg_variant_get_string = dlsym(library, "g_variant_get_string");

    DConfClient*(*ffdconf_client_new)(void) = dlsym(library, "dconf_client_new");

    if(
        data.ffdconf_client_read_full == NULL ||
        data.ffg_variant_get_string == NULL ||
        (data.client = ffdconf_client_new()) == NULL
    ) {
        pthread_mutex_unlock(&mutex);
        dlclose(library);
        return NULL;
    }

    pthread_mutex_unlock(&mutex);
    return getDConfValue(&data, key);
}

typedef struct GSettingsData
{
    GSettingsSchemaSource* schemaSource;
    GSettingsSchema*(*ffg_settings_schema_source_lookup)(GSettingsSchemaSource*, const gchar*, gboolean);
    gboolean(*ffg_settings_schema_has_key)(GSettingsSchema*, const gchar*);
    GSettings*(*ffg_settings_new_full)(GSettingsSchema*, GSettingsBackend*, const gchar*);
    GVariant*(*ffg_settings_get_value)(GSettings*, const gchar*);
    GVariant*(*ffg_settings_get_user_value)(GSettings*, const gchar*);
    GVariant*(*ffg_settings_get_default_value)(GSettings*, const gchar*);
    const gchar*(*ffg_variant_get_string)(GVariant*, gsize*);
    gboolean(*ffg_variant_get_boolean)(GVariant*);
} GSettingsData;

static FFvariant getGSettingsValue(GSettingsData* data, const char* schemaName, const char* path, const char* key, FFvarianttype type)
{
    FFvariant result;
    result.strValue = NULL;
    result.set = false;

    if(data->schemaSource == NULL)
        return result;

    GSettingsSchema* schema = data->ffg_settings_schema_source_lookup(data->schemaSource, schemaName, true);
    if(schema == NULL)
        return result;

    if(data->ffg_settings_schema_has_key(schema, key) == 0)
        return result;

    GSettings* settings = data->ffg_settings_new_full(schema, NULL, path);
    if(settings == NULL)
        return result;

    GVariant* variant = data->ffg_settings_get_value(settings, key);

    if(variant == NULL)
        variant = data->ffg_settings_get_user_value(settings, key);

    if(variant == NULL)
        variant = data->ffg_settings_get_default_value(settings, key);

    if(variant == NULL)
        return result;

    if(type == FF_VARIANT_TYPE_STRING && data->ffg_variant_get_string != NULL)
    {
        result.strValue = data->ffg_variant_get_string(variant, NULL);
        result.set = result.strValue != NULL;
    }
    else if(type == FF_VARIANT_TYPE_BOOL && data->ffg_variant_get_boolean != NULL)
    {
        result.boolValue = data->ffg_variant_get_boolean(variant);
        result.set = true;
    }

    return result;
}

FFvariant ffSettingsGetGsettings(FFinstance* instance, const char* schemaName, const char* path, const char* key, FFvarianttype type)
{
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    static bool init = false;

    static GSettingsData data;

    pthread_mutex_lock(&mutex);

    if(init)
    {
        pthread_mutex_unlock(&mutex);
        return getGSettingsValue(&data, schemaName, path, key, type);
    }

    init = true;

    data.schemaSource = NULL; //error indicator

    FFvariant errorResult;
    errorResult.set = false;
    errorResult.strValue = NULL;

    void* library = dlopen(instance->config.libGIO.length == 0 ? "libgio-2.0.so" : instance->config.libGIO.chars, RTLD_LAZY);
    if(library == NULL)
    {
        pthread_mutex_unlock(&mutex);
        return errorResult;
    }

    data.ffg_settings_schema_source_lookup = dlsym(library, "g_settings_schema_source_lookup");
    data.ffg_settings_schema_has_key       = dlsym(library, "g_settings_schema_has_key");
    data.ffg_settings_new_full             = dlsym(library, "g_settings_new_full");
    data.ffg_settings_get_value            = dlsym(library, "g_settings_get_value");
    data.ffg_settings_get_user_value       = dlsym(library, "g_settings_get_user_value");
    data.ffg_settings_get_default_value    = dlsym(library, "g_settings_get_default_value");
    data.ffg_variant_get_string            = dlsym(library, "g_variant_get_string");
    data.ffg_variant_get_boolean           = dlsym(library, "g_variant_get_boolean");

    GSettingsSchemaSource*(*ffg_settings_schema_source_get_default)(void) = dlsym(library, "g_settings_schema_source_get_default");

    if(
        data.ffg_settings_schema_source_lookup == NULL ||
        data.ffg_settings_schema_has_key       == NULL ||
        data.ffg_settings_new_full             == NULL ||
        data.ffg_settings_get_value            == NULL ||
        data.ffg_settings_get_user_value       == NULL ||
        data.ffg_settings_get_default_value    == NULL ||
        ffg_settings_schema_source_get_default == NULL ||
        (data.schemaSource = ffg_settings_schema_source_get_default()) == NULL
    ) {
        pthread_mutex_unlock(&mutex);
        dlclose(library);
        return errorResult;
    }

    pthread_mutex_unlock(&mutex);
    return getGSettingsValue(&data, schemaName, path, key, type);
}

const char* ffSettingsGet(FFinstance* instance, const char* dconfKey, const char* gsettingsSchemaName, const char* gsettingsPath, const char* gsettingsKey)
{
    const char* dconf = ffSettingsGetDConf(instance, dconfKey);
    if(dconf != NULL)
        return dconf;

    return ffSettingsGetGsettings(instance, gsettingsSchemaName, gsettingsPath, gsettingsKey, FF_VARIANT_TYPE_STRING).strValue;
}

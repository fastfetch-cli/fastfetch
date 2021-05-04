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

    void* library = dlopen(instance->config.libDConf.chars == NULL ? "libdconf.so" : instance->config.libDConf.chars, RTLD_LAZY);
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
} GSettingsData;

static const char* getGSettingsValue(GSettingsData* data, const char* schemaName, const char* path, const char* key)
{
    if(data->schemaSource == NULL)
        return NULL;

    GSettingsSchema* schema = data->ffg_settings_schema_source_lookup(data->schemaSource, schemaName, true);
    if(schema == NULL)
        return NULL;

    if(data->ffg_settings_schema_has_key(schema, key) == 0)
        return NULL;

    GSettings* settings = data->ffg_settings_new_full(schema, NULL, path);
    if(settings == NULL)
        return NULL;

    GVariant* variant = data->ffg_settings_get_value(settings, key);

    if(variant == NULL)
        variant = data->ffg_settings_get_user_value(settings, key);

    if(variant == NULL)
        variant = data->ffg_settings_get_default_value(settings, key);

    if(variant == NULL)
        return NULL;

    return data->ffg_variant_get_string(variant, NULL);
}

const char* ffSettingsGetGsettingsPath(FFinstance* instance, const char* schemaName, const char* path, const char* key)
{
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    static bool init = false;

    static GSettingsData data;

    pthread_mutex_lock(&mutex);

    if(init)
    {
        pthread_mutex_unlock(&mutex);
        return getGSettingsValue(&data, schemaName, path, key);
    }

    init = true;

    data.schemaSource = NULL; //error indicator

    void* library = dlopen(instance->config.libGIO.length == 0 ? "libgio-2.0.so" : instance->config.libGIO.chars, RTLD_LAZY);
    if(library == NULL)
    {
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    data.ffg_settings_schema_source_lookup = dlsym(library, "g_settings_schema_source_lookup");
    data.ffg_settings_schema_has_key       = dlsym(library, "g_settings_schema_has_key");
    data.ffg_settings_new_full             = dlsym(library, "g_settings_new_full");
    data.ffg_settings_get_value            = dlsym(library, "g_settings_get_value");
    data.ffg_settings_get_user_value       = dlsym(library, "g_settings_get_user_value");
    data.ffg_settings_get_default_value    = dlsym(library, "g_settings_get_default_value");
    data.ffg_variant_get_string            = dlsym(library, "g_variant_get_string");

    GSettingsSchemaSource*(*ffg_settings_schema_source_get_default)(void) = dlsym(library, "g_settings_schema_source_get_default");

    if(
        data.ffg_settings_schema_source_lookup == NULL ||
        data.ffg_settings_schema_has_key       == NULL ||
        data.ffg_settings_new_full             == NULL ||
        data.ffg_settings_get_value            == NULL ||
        data.ffg_settings_get_user_value       == NULL ||
        data.ffg_settings_get_default_value    == NULL ||
        data.ffg_variant_get_string            == NULL ||
        ffg_settings_schema_source_get_default == NULL ||
        (data.schemaSource = ffg_settings_schema_source_get_default()) == NULL
    ) {
        pthread_mutex_unlock(&mutex);
        dlclose(library);
        return NULL;
    }

    pthread_mutex_unlock(&mutex);
    return getGSettingsValue(&data, schemaName, path, key);
}

const char* ffSettingsGetGSettings(FFinstance* instance, const char* schemaName, const char* key)
{
    return ffSettingsGetGsettingsPath(instance, schemaName, NULL, key);
}

const char* ffSettingsGetPath(FFinstance* instance, const char* dconfKey, const char* gsettingsSchemaName, const char* gsettingsPath, const char* gsettingsKey)
{
    const char* dconf = ffSettingsGetDConf(instance, dconfKey);
    if(dconf != NULL)
        return dconf;

    return ffSettingsGetGsettingsPath(instance, gsettingsSchemaName, gsettingsPath, gsettingsKey);
}

const char* ffSettingsGet(FFinstance* instance, const char* dconfKey, const char* gsettingsSchemaName, const char* gsettingsKey)
{
    return ffSettingsGetPath(instance, dconfKey, gsettingsSchemaName, NULL, gsettingsKey);
}

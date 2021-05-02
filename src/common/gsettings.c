#include "fastfetch.h"

#include <dlfcn.h>
#include <pthread.h>
#include <gio/gio.h>
#include <dconf.h>

typedef struct DConfData
{
    void* library;
    GVariant*(*ffdconf_client_read_full)(DConfClient*, const gchar*, DConfReadFlags, const GQueue*);
    const gchar*(*ffg_variant_get_string)(GVariant*, gsize*);
    DConfClient* client;
} DConfData;

typedef struct GSettingsData
{
    void* library;
    GSettingsSchemaSource* schemaSource;
    GSettingsSchema*(*ffg_settings_schema_source_lookup)(GSettingsSchemaSource*, const gchar*, gboolean);
    gboolean(*ffg_settings_schema_has_key)(GSettingsSchema*, const gchar*);
    GSettings*(*ffg_settings_new_full)(GSettingsSchema*, GSettingsBackend*, const gchar*);
    GVariant*(*ffg_settings_get_value)(GSettings*, const gchar*);
    GVariant*(*ffg_settings_get_user_value)(GSettings*, const gchar*);
    GVariant*(*ffg_settings_get_default_value)(GSettings*, const gchar*);
    const gchar*(*ffg_variant_get_string)(GVariant*, gsize*);
} GSettingsData;

static int transformGSettingsToDConf(int c)
{
    return c == '.' ? '/' : c;
}

static const char* getDConfValue(DConfData* data, const char* schemaName, const char* path, const char* key)
{
    if(data->client == NULL)
        return NULL;

    FFstrbuf dconfStyleKey;
    ffStrbufInitA(&dconfStyleKey, 64);
    ffStrbufAppendC(&dconfStyleKey, '/');
    ffStrbufAppendTransformS(&dconfStyleKey, schemaName, transformGSettingsToDConf);

    if(path != NULL && *path != '/')
        ffStrbufAppendC(&dconfStyleKey, '/');

    ffStrbufAppendTransformS(&dconfStyleKey, path, transformGSettingsToDConf);

    if(dconfStyleKey.chars[dconfStyleKey.length - 1] != '/')
        ffStrbufAppendC(&dconfStyleKey, '/');

    ffStrbufAppendS(&dconfStyleKey, key);

    GVariant* variant = data->ffdconf_client_read_full(data->client, dconfStyleKey.chars, DCONF_READ_FLAGS_NONE, NULL);

    if(variant == NULL)
        variant = data->ffdconf_client_read_full(data->client, dconfStyleKey.chars, DCONF_READ_USER_VALUE, NULL);

    if(variant == NULL)
        variant = data->ffdconf_client_read_full(data->client, dconfStyleKey.chars, DCONF_READ_DEFAULT_VALUE, NULL);

    ffStrbufDestroy(&dconfStyleKey);

    if(variant != NULL)
        return data->ffg_variant_get_string(variant, NULL);

    return NULL;
}

static const char* dconfGetValue(FFinstance* instance, const char* schemaName, const char* path, const char* key)
{
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    static bool init = false;

    static DConfData data;

    pthread_mutex_lock(&mutex);

    if(init)
    {
        pthread_mutex_unlock(&mutex);
        return getDConfValue(&data, schemaName, path, key);
    }

    init = true;

    data.client = NULL; //error indicator

    data.library = dlopen(instance->config.libDConf.chars == NULL ? "libdconf.so" : instance->config.libDConf.chars, RTLD_LAZY);
    if(data.library == NULL)
    {
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    data.ffdconf_client_read_full = dlsym(data.library, "dconf_client_read_full");
    data.ffg_variant_get_string = dlsym(data.library, "g_variant_get_string");

    DConfClient*(*ffdconf_client_new)(void) = dlsym(data.library, "dconf_client_new");

    if(
        data.ffdconf_client_read_full == NULL ||
        data.ffg_variant_get_string == NULL ||
        (data.client = ffdconf_client_new()) == NULL
    ) {
        pthread_mutex_unlock(&mutex);
        dlclose(data.library);
        return NULL;
    }

    pthread_mutex_unlock(&mutex);
    return getDConfValue(&data, schemaName, path, key);
}

static const char* getGSettingsValue(FFinstance* instance, GSettingsData* data, const char* schemaName, const char* path, const char* key)
{
    if(data->schemaSource == NULL)
        return dconfGetValue(instance, schemaName, path, key);

    GSettingsSchema* schema = data->ffg_settings_schema_source_lookup(data->schemaSource, schemaName, true);
    if(schema == NULL)
        return dconfGetValue(instance, schemaName, path, key);

    if(data->ffg_settings_schema_has_key(schema, key) == 0)
        return dconfGetValue(instance, schemaName, path, key);

    GSettings* settings = data->ffg_settings_new_full(schema, NULL, path);
    if(settings == NULL)
        return dconfGetValue(instance, schemaName, path, key);

    GVariant* variant = data->ffg_settings_get_value(settings, key);

    if(variant == NULL)
        variant = data->ffg_settings_get_user_value(settings, key);

    if(variant == NULL)
        variant = data->ffg_settings_get_default_value(settings, key);

    if(variant == NULL)
        return dconfGetValue(instance, schemaName, path, key);

    const char* value = data->ffg_variant_get_string(variant, NULL);

    if(value == NULL || *value == '\0')
        return dconfGetValue(instance, schemaName, path, key);

    return value;
}

const char* ffGSettingsGetValuePath(FFinstance* instance, const char* schemaName, const char* path, const char* key)
{
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    static bool init = false;

    static GSettingsData data;

    pthread_mutex_lock(&mutex);

    if(init)
    {
        pthread_mutex_unlock(&mutex);
        return getGSettingsValue(instance, &data, schemaName, path, key);
    }

    init = true;

    data.schemaSource = NULL; //error indicator

    data.library = dlopen(instance->config.libGIO.length == 0 ? "libgio-2.0.so" : instance->config.libGIO.chars, RTLD_LAZY);
    if(data.library == NULL)
    {
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    data.ffg_settings_schema_source_lookup = dlsym(data.library, "g_settings_schema_source_lookup");
    data.ffg_settings_schema_has_key       = dlsym(data.library, "g_settings_schema_has_key");
    data.ffg_settings_new_full             = dlsym(data.library, "g_settings_new_full");
    data.ffg_settings_get_value            = dlsym(data.library, "g_settings_get_value");
    data.ffg_settings_get_user_value       = dlsym(data.library, "g_settings_get_user_value");
    data.ffg_settings_get_default_value    = dlsym(data.library, "g_settings_get_default_value");
    data.ffg_variant_get_string            = dlsym(data.library, "g_variant_get_string");

    GSettingsSchemaSource*(*ffg_settings_schema_source_get_default)(void) = dlsym(data.library, "g_settings_schema_source_get_default");

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
        dlclose(data.library);
        return NULL;
    }

    pthread_mutex_unlock(&mutex);
    return getGSettingsValue(instance, &data, schemaName, path, key);
}

const char* ffGSettingsGetValue(FFinstance* instance, const char* schemaName, const char* key)
{
    return ffGSettingsGetValuePath(instance, schemaName, NULL, key);
}

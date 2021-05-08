#include "fastfetch.h"

#include <dlfcn.h>
#include <pthread.h>
#include <gio/gio.h>
#include <dconf.h>

typedef struct GVariantGetters
{
    const gchar*(*ffg_variant_get_string)(GVariant*, gsize*);
    gboolean(*ffg_variant_get_boolean)(GVariant*);
} GVariantGetters;

static inline void initGVariantGetters(void* library, GVariantGetters* variantGetters)
{
    variantGetters->ffg_variant_get_string  = dlsym(library, "g_variant_get_string");
    variantGetters->ffg_variant_get_boolean = dlsym(library, "g_variant_get_boolean");
}

#define FF_VARIANT_NULL (FFvariant)(const char*)NULL

static FFvariant getGVariantValue(GVariant* variant, FFvarianttype type, GVariantGetters* variantGetters)
{
    if(variant == NULL)
        return FF_VARIANT_NULL;

    if(type == FF_VARIANT_TYPE_STRING && variantGetters->ffg_variant_get_string != NULL)
        return (FFvariant) variantGetters->ffg_variant_get_string(variant, NULL);

    if(type == FF_VARIANT_TYPE_BOOL && variantGetters->ffg_variant_get_boolean != NULL)
        return (FFvariant) { .boolValue = (bool) variantGetters->ffg_variant_get_boolean(variant), .boolValueSet = true};

    return FF_VARIANT_NULL;
}

typedef struct DConfData
{
    GVariant*(*ffdconf_client_read_full)(DConfClient*, const gchar*, DConfReadFlags, const GQueue*);
    GVariantGetters variantGetters;
    DConfClient* client;
} DConfData;

static FFvariant getDConfValue(DConfData* data, const char* key, FFvarianttype type)
{
    if(data->client == NULL)
        return FF_VARIANT_NULL;

    GVariant* variant = data->ffdconf_client_read_full(data->client, key, DCONF_READ_FLAGS_NONE, NULL);

    if(variant == NULL)
        variant = data->ffdconf_client_read_full(data->client, key, DCONF_READ_USER_VALUE, NULL);

    if(variant == NULL)
        variant = data->ffdconf_client_read_full(data->client, key, DCONF_READ_DEFAULT_VALUE, NULL);

    return getGVariantValue(variant, type, &data->variantGetters);
}

FFvariant ffSettingsGetDConf(FFinstance* instance, const char* key, FFvarianttype type)
{
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    static bool init = false;

    static DConfData data;

    pthread_mutex_lock(&mutex);

    if(init)
    {
        pthread_mutex_unlock(&mutex);
        return getDConfValue(&data, key, type);
    }

    init = true;

    data.client = NULL; //error indicator

    void* library = dlopen(instance->config.libDConf.length == 0 ? "libdconf.so" : instance->config.libDConf.chars, RTLD_LAZY);
    if(library == NULL)
    {
        pthread_mutex_unlock(&mutex);
        return FF_VARIANT_NULL;
    }

    data.ffdconf_client_read_full = dlsym(library, "dconf_client_read_full");
    initGVariantGetters(library, &data.variantGetters);

    DConfClient*(*ffdconf_client_new)(void) = dlsym(library, "dconf_client_new");

    if(
        data.ffdconf_client_read_full == NULL ||
        (data.client = ffdconf_client_new()) == NULL
    ) {
        pthread_mutex_unlock(&mutex);
        dlclose(library);
        return FF_VARIANT_NULL;
    }

    pthread_mutex_unlock(&mutex);
    return getDConfValue(&data, key, type);
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
    GVariantGetters variantGetters;
} GSettingsData;

static FFvariant getGSettingsValue(GSettingsData* data, const char* schemaName, const char* path, const char* key, FFvarianttype type)
{
    if(data->schemaSource == NULL)
        return FF_VARIANT_NULL;

    GSettingsSchema* schema = data->ffg_settings_schema_source_lookup(data->schemaSource, schemaName, true);
    if(schema == NULL)
        return FF_VARIANT_NULL;

    if(data->ffg_settings_schema_has_key(schema, key) == 0)
        return FF_VARIANT_NULL;

    GSettings* settings = data->ffg_settings_new_full(schema, NULL, path);
    if(settings == NULL)
        return FF_VARIANT_NULL;

    GVariant* variant = data->ffg_settings_get_value(settings, key);

    if(variant == NULL)
        variant = data->ffg_settings_get_user_value(settings, key);

    if(variant == NULL)
        variant = data->ffg_settings_get_default_value(settings, key);

    return getGVariantValue(variant, type, &data->variantGetters);
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

    void* library = dlopen(instance->config.libGIO.length == 0 ? "libgio-2.0.so" : instance->config.libGIO.chars, RTLD_LAZY);
    if(library == NULL)
    {
        pthread_mutex_unlock(&mutex);
        return FF_VARIANT_NULL;
    }

    data.ffg_settings_schema_source_lookup = dlsym(library, "g_settings_schema_source_lookup");
    data.ffg_settings_schema_has_key       = dlsym(library, "g_settings_schema_has_key");
    data.ffg_settings_new_full             = dlsym(library, "g_settings_new_full");
    data.ffg_settings_get_value            = dlsym(library, "g_settings_get_value");
    data.ffg_settings_get_user_value       = dlsym(library, "g_settings_get_user_value");
    data.ffg_settings_get_default_value    = dlsym(library, "g_settings_get_default_value");
    initGVariantGetters(library, &data.variantGetters);

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
        return FF_VARIANT_NULL;
    }

    pthread_mutex_unlock(&mutex);
    return getGSettingsValue(&data, schemaName, path, key, type);
}

FFvariant ffSettingsGet(FFinstance* instance, const char* dconfKey, const char* gsettingsSchemaName, const char* gsettingsPath, const char* gsettingsKey, FFvarianttype type)
{
    FFvariant dconf = ffSettingsGetDConf(instance, dconfKey, type);
    if(dconf.strValue != NULL)
        return dconf;

    return ffSettingsGetGsettings(instance, gsettingsSchemaName, gsettingsPath, gsettingsKey, FF_VARIANT_TYPE_STRING);
}

const char* ffSettingsGetStr(FFinstance* instance, const char* dconfKey, const char* gsettingsSchemaName, const char* gsettingsPath, const char* gsettingsKey)
{
    return ffSettingsGet(instance, dconfKey, gsettingsSchemaName, gsettingsPath, gsettingsKey, FF_VARIANT_TYPE_STRING).strValue;
}

#undef FF_VARIANT_NULL

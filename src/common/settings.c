#include "fastfetch.h"

#include <dlfcn.h>
#include <pthread.h>
#include <dconf.h> // Also included gio/gio.h

typedef void* DynamicLibrary;

#define FF_VARIANT_NULL ((FFvariant){.strValue = NULL})

#define FF_LIBRARY_LOAD(libraryNameUser, libraryNameDefault, mutex) dlopen(libraryNameUser.length == 0 ? libraryNameDefault : libraryNameUser.chars, RTLD_LAZY); \
    if(dlerror() != NULL) { \
        pthread_mutex_unlock(&mutex); \
        return FF_VARIANT_NULL; \
    }

#define FF_LIBRARY_ERROR_RETURN(library, mutex) { \
    pthread_mutex_unlock(&mutex); \
    dlclose(library); \
    return FF_VARIANT_NULL; \
}

#define FF_LIBRARY_LOAD_SYMBOL(library, symbolName, mutex) dlsym(library, symbolName); \
    if(dlerror() != NULL) \
        FF_LIBRARY_ERROR_RETURN(library, mutex)

typedef struct GVariantGetters
{
    const gchar*(*ffg_variant_get_string)(GVariant*, gsize*);
    gboolean(*ffg_variant_get_boolean)(GVariant*);
} GVariantGetters;

#define FF_LIBRARY_GVARIANT_GETTERS_INIT(library, object, mutex) \
    object.ffg_variant_get_string = FF_LIBRARY_LOAD_SYMBOL(library, "g_variant_get_string", mutex); \
    object.ffg_variant_get_boolean = FF_LIBRARY_LOAD_SYMBOL(library, "g_variant_get_boolean", mutex);

static FFvariant getGVariantValue(GVariant* variant, FFvarianttype type, GVariantGetters* variantGetters)
{
    if(variant == NULL)
        return FF_VARIANT_NULL;

    if(type == FF_VARIANT_TYPE_STRING)
        return (FFvariant) variantGetters->ffg_variant_get_string(variant, NULL);

    if(type == FF_VARIANT_TYPE_BOOL)
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
    if(variant != NULL)
        return getGVariantValue(variant, type, &data->variantGetters);

    variant = data->ffdconf_client_read_full(data->client, key, DCONF_READ_USER_VALUE, NULL);
    if(variant != NULL)
        return getGVariantValue(variant, type, &data->variantGetters);

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

    DynamicLibrary library = FF_LIBRARY_LOAD(instance->config.libDConf, "libdocnf.so", mutex);

    data.ffdconf_client_read_full = FF_LIBRARY_LOAD_SYMBOL(library, "dconf_client_read_full", mutex);
    FF_LIBRARY_GVARIANT_GETTERS_INIT(library, data.variantGetters, mutex);

    DConfClient*(*ffdconf_client_new)(void) = FF_LIBRARY_LOAD_SYMBOL(library, "dconf_client_new", mutex);
    if((data.client = ffdconf_client_new()) == NULL)
        FF_LIBRARY_ERROR_RETURN(library, mutex);

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
    if(variant != NULL)
        return getGVariantValue(variant, type, &data->variantGetters);

    variant = data->ffg_settings_get_user_value(settings, key);
    if(variant != NULL)
        return getGVariantValue(variant, type, &data->variantGetters);

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

    DynamicLibrary library = FF_LIBRARY_LOAD(instance->config.libGIO, "libgio-2.0.so", mutex);

    data.ffg_settings_schema_source_lookup = FF_LIBRARY_LOAD_SYMBOL(library, "g_settings_schema_source_lookup", mutex);
    data.ffg_settings_schema_has_key = FF_LIBRARY_LOAD_SYMBOL(library, "g_settings_schema_has_key", mutex);
    data.ffg_settings_new_full = FF_LIBRARY_LOAD_SYMBOL(library, "g_settings_new_full", mutex);
    data.ffg_settings_get_value = FF_LIBRARY_LOAD_SYMBOL(library, "g_settings_get_value", mutex);
    data.ffg_settings_get_user_value = FF_LIBRARY_LOAD_SYMBOL(library, "g_settings_get_user_value", mutex);
    data.ffg_settings_get_default_value = FF_LIBRARY_LOAD_SYMBOL(library, "g_settings_get_default_value", mutex);
    FF_LIBRARY_GVARIANT_GETTERS_INIT(library, data.variantGetters, mutex);

    GSettingsSchemaSource*(*ffg_settings_schema_source_get_default)(void) = FF_LIBRARY_LOAD_SYMBOL(library, "g_settings_schema_source_get_default", mutex);
    if((data.schemaSource = ffg_settings_schema_source_get_default()) == NULL)
        FF_LIBRARY_ERROR_RETURN(library, mutex);

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

typedef struct _XfconfChannel XfconfChannel; // /usr/include/xfce4/xfconf-0/xfconf/xfconf-channel.h#L39

typedef struct XFConfData
{
    bool init;
    XfconfChannel*(*ffxfconf_channel_get)(const gchar*);
    gboolean(*ffxfconf_channel_has_property)(XfconfChannel*, const gchar*);
    gchar*(*ffxfconf_channel_get_string)(XfconfChannel*, const gchar*, const gchar*);
    gboolean(*ffxfconf_channel_get_bool)(XfconfChannel*, const gchar*, gboolean);
} XFConfData;

static FFvariant getXFConfValue(XFConfData* data, const char* channelName, const char* propertyName, FFvarianttype type)
{
    if(!data->init)
        return FF_VARIANT_NULL;

    XfconfChannel* channel = data->ffxfconf_channel_get(channelName); // Never fails according to documentation but rather returns an empty channel

    if(!data->ffxfconf_channel_has_property(channel, propertyName))
        return FF_VARIANT_NULL;

    if(type == FF_VARIANT_TYPE_STRING)
        return (FFvariant) {.strValue = data->ffxfconf_channel_get_string(channel, propertyName, NULL)};

    if(type == FF_VARIANT_TYPE_BOOL)
        return (FFvariant) {.boolValue = data->ffxfconf_channel_get_bool(channel, propertyName, false), .boolValueSet = true};

    return FF_VARIANT_NULL;
}

FFvariant ffSettingsGetXFConf(FFinstance* instance, const char* channelName, const char* propertyName, FFvarianttype type)
{
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    static bool init = false;

    static XFConfData data;

    pthread_mutex_lock(&mutex);

    if(init)
    {
        pthread_mutex_unlock(&mutex);
        return getXFConfValue(&data, channelName, propertyName, type);
    }
    init = true;

    data.init = false; //error indicator

    DynamicLibrary library = FF_LIBRARY_LOAD(instance->config.libXFConf, "libxfconf-0.so", mutex);

    data.ffxfconf_channel_get = FF_LIBRARY_LOAD_SYMBOL(library, "xfconf_channel_get", mutex);
    data.ffxfconf_channel_has_property = FF_LIBRARY_LOAD_SYMBOL(library, "xfconf_channel_has_property", mutex);
    data.ffxfconf_channel_get_string = FF_LIBRARY_LOAD_SYMBOL(library, "xfconf_channel_get_string", mutex);
    data.ffxfconf_channel_get_bool = FF_LIBRARY_LOAD_SYMBOL(library, "xfconf_channel_get_bool", mutex);

    gboolean(*ffxfconf_init)(GError **) = FF_LIBRARY_LOAD_SYMBOL(library, "xfconf_init", mutex);
    if((data.init = ffxfconf_init(NULL)) == FALSE)
        FF_LIBRARY_ERROR_RETURN(library, mutex);

    pthread_mutex_unlock(&mutex);
    return getXFConfValue(&data, channelName, propertyName, type);
}

#undef FF_VARIANT_NULL

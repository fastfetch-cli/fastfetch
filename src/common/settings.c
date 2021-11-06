#include "fastfetch.h"

#include <dlfcn.h>
#include <pthread.h>

typedef void* DynamicLibrary;

#define FF_VARIANT_NULL ((FFvariant){.strValue = NULL})

#define FF_LIBRARY_LOAD(libraryNameUser, libraryNameDefault, mutex, returnValue) dlopen(libraryNameUser.length == 0 ? libraryNameDefault : libraryNameUser.chars, RTLD_LAZY); \
    if(dlerror() != NULL) { \
        pthread_mutex_unlock(&mutex); \
        return returnValue; \
    }

#define FF_LIBRARY_ERROR_RETURN(library, mutex, returnValue) { \
    pthread_mutex_unlock(&mutex); \
    dlclose(library); \
    return returnValue; \
}

#define FF_LIBRARY_LOAD_SYMBOL(library, symbolName, mutex, returnValue) dlsym(library, symbolName); \
    if(dlerror() != NULL) \
        FF_LIBRARY_ERROR_RETURN(library, mutex, returnValue)

#ifdef FF_HAVE_GIO
#include <gio/gio.h>

typedef struct GVariantGetters
{
    const gchar*(*ffg_variant_get_string)(GVariant*, gsize*);
    gboolean(*ffg_variant_get_boolean)(GVariant*);
    gint32(*ffg_variant_get_int32)(GVariant*);
} GVariantGetters;

#define FF_LIBRARY_GVARIANT_GETTERS_INIT(library, object, mutex) \
    object.ffg_variant_get_string = FF_LIBRARY_LOAD_SYMBOL(library, "g_variant_get_string", mutex, FF_VARIANT_NULL); \
    object.ffg_variant_get_boolean = FF_LIBRARY_LOAD_SYMBOL(library, "g_variant_get_boolean", mutex, FF_VARIANT_NULL); \
    object.ffg_variant_get_int32 = FF_LIBRARY_LOAD_SYMBOL(library, "g_variant_get_int32", mutex, FF_VARIANT_NULL)

static FFvariant getGVariantValue(GVariant* variant, FFvarianttype type, GVariantGetters* variantGetters)
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

FFvariant ffSettingsGetGSettings(FFinstance* instance, const char* schemaName, const char* path, const char* key, FFvarianttype type)
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

    DynamicLibrary library = FF_LIBRARY_LOAD(instance->config.libGIO, "libgio-2.0.so", mutex, FF_VARIANT_NULL);

    data.ffg_settings_schema_source_lookup = FF_LIBRARY_LOAD_SYMBOL(library, "g_settings_schema_source_lookup", mutex, FF_VARIANT_NULL);
    data.ffg_settings_schema_has_key = FF_LIBRARY_LOAD_SYMBOL(library, "g_settings_schema_has_key", mutex, FF_VARIANT_NULL);
    data.ffg_settings_new_full = FF_LIBRARY_LOAD_SYMBOL(library, "g_settings_new_full", mutex, FF_VARIANT_NULL);
    data.ffg_settings_get_value = FF_LIBRARY_LOAD_SYMBOL(library, "g_settings_get_value", mutex, FF_VARIANT_NULL);
    data.ffg_settings_get_user_value = FF_LIBRARY_LOAD_SYMBOL(library, "g_settings_get_user_value", mutex, FF_VARIANT_NULL);
    data.ffg_settings_get_default_value = FF_LIBRARY_LOAD_SYMBOL(library, "g_settings_get_default_value", mutex, FF_VARIANT_NULL);
    FF_LIBRARY_GVARIANT_GETTERS_INIT(library, data.variantGetters, mutex);

    GSettingsSchemaSource*(*ffg_settings_schema_source_get_default)(void) = FF_LIBRARY_LOAD_SYMBOL(library, "g_settings_schema_source_get_default", mutex, FF_VARIANT_NULL);
    if((data.schemaSource = ffg_settings_schema_source_get_default()) == NULL)
        FF_LIBRARY_ERROR_RETURN(library, mutex, FF_VARIANT_NULL);

    pthread_mutex_unlock(&mutex);
    return getGSettingsValue(&data, schemaName, path, key, type);
}

#else //FF_HAVE_GIO
FFvariant ffSettingsGetGSettings(FFinstance* instance, const char* schemaName, const char* path, const char* key, FFvarianttype type)
{
    FF_UNUSED(instance, schemaName, path, key, type)
    return FF_VARIANT_NULL;
}
#endif //FF_HAVE_GIO

#ifdef FF_HAVE_DCONF
#include <dconf.h>

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

    DynamicLibrary library = FF_LIBRARY_LOAD(instance->config.libDConf, "libdconf.so", mutex, FF_VARIANT_NULL);

    data.ffdconf_client_read_full = FF_LIBRARY_LOAD_SYMBOL(library, "dconf_client_read_full", mutex, FF_VARIANT_NULL);
    FF_LIBRARY_GVARIANT_GETTERS_INIT(library, data.variantGetters, mutex);

    DConfClient*(*ffdconf_client_new)(void) = FF_LIBRARY_LOAD_SYMBOL(library, "dconf_client_new", mutex, FF_VARIANT_NULL);
    if((data.client = ffdconf_client_new()) == NULL)
        FF_LIBRARY_ERROR_RETURN(library, mutex, FF_VARIANT_NULL);

    pthread_mutex_unlock(&mutex);
    return getDConfValue(&data, key, type);
}

#else //FF_HAVE_DCONF
FFvariant ffSettingsGetDConf(FFinstance* instance, const char* key, FFvarianttype type)
{
    FF_UNUSED(instance, key, type)
    return FF_VARIANT_NULL;
}
#endif //FF_HAVE_DCONF

FFvariant ffSettingsGet(FFinstance* instance, const char* dconfKey, const char* gsettingsSchemaName, const char* gsettingsPath, const char* gsettingsKey, FFvarianttype type)
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
    bool init;
    XfconfChannel*(*ffxfconf_channel_get)(const gchar*);
    gboolean(*ffxfconf_channel_has_property)(XfconfChannel*, const gchar*);
    gchar*(*ffxfconf_channel_get_string)(XfconfChannel*, const gchar*, const gchar*);
    gboolean(*ffxfconf_channel_get_bool)(XfconfChannel*, const gchar*, gboolean);
    gint32(*ffxfconf_channel_get_int)(XfconfChannel*, const gchar*, gint32);
} XFConfData;

static FFvariant getXFConfValue(XFConfData* data, const char* channelName, const char* propertyName, FFvarianttype type)
{
    if(!data->init)
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

    DynamicLibrary library = FF_LIBRARY_LOAD(instance->config.libXFConf, "libxfconf-0.so", mutex, FF_VARIANT_NULL);

    data.ffxfconf_channel_get = FF_LIBRARY_LOAD_SYMBOL(library, "xfconf_channel_get", mutex, FF_VARIANT_NULL);
    data.ffxfconf_channel_has_property = FF_LIBRARY_LOAD_SYMBOL(library, "xfconf_channel_has_property", mutex, FF_VARIANT_NULL);
    data.ffxfconf_channel_get_string = FF_LIBRARY_LOAD_SYMBOL(library, "xfconf_channel_get_string", mutex, FF_VARIANT_NULL);
    data.ffxfconf_channel_get_bool = FF_LIBRARY_LOAD_SYMBOL(library, "xfconf_channel_get_bool", mutex, FF_VARIANT_NULL);
    data.ffxfconf_channel_get_int = FF_LIBRARY_LOAD_SYMBOL(library, "xfconf_channel_get_int", mutex, FF_VARIANT_NULL);

    gboolean(*ffxfconf_init)(GError **) = FF_LIBRARY_LOAD_SYMBOL(library, "xfconf_init", mutex, FF_VARIANT_NULL);
    if((data.init = ffxfconf_init(NULL)) == FALSE)
        FF_LIBRARY_ERROR_RETURN(library, mutex, FF_VARIANT_NULL);

    pthread_mutex_unlock(&mutex);
    return getXFConfValue(&data, channelName, propertyName, type);
}
#else //FF_HAVE_XFCONF
FFvariant ffSettingsGetXFConf(FFinstance* instance, const char* channelName, const char* propertyName, FFvarianttype type)
{
    FF_UNUSED(instance, channelName, propertyName, type)
    return FF_VARIANT_NULL;
}
#endif //FF_HAVE_XFCONF

#ifdef FF_HAVE_RPM
#include <rpm/rpmlib.h>
#include <rpm/rpmts.h>
#include <rpm/rpmdb.h>

typedef struct LibrpmData
{
    __typeof__(&rpmReadConfigFiles) ffrpmReadConfigFiles;
    __typeof__(&rpmtsCreate) ffrpmtsCreate;
    __typeof__(&rpmtsInitIterator) ffrpmtsInitIterator;
    __typeof__(&rpmdbGetIteratorCount) ffrpmdbGetIteratorCount;
    __typeof__(&rpmdbFreeIterator) ffrpmdbFreeIterator;
    __typeof__(&rpmtsFree) ffrpmtsFree;
} LibrpmData;

uint32_t getRpmPackageCount(LibrpmData* data) {
    uint32_t count = 0;
    rpmts ts = NULL;
    rpmdbMatchIterator mi = NULL;

    if (data->ffrpmReadConfigFiles(NULL, NULL)) goto exit;
    if (!(ts = data->ffrpmtsCreate())) goto exit;
    if (!(mi = data->ffrpmtsInitIterator(ts, RPMDBI_LABEL, NULL, 0))) goto exit;
    count = data->ffrpmdbGetIteratorCount(mi);

exit:
    if (mi) data->ffrpmdbFreeIterator(mi);
    if (ts) data->ffrpmtsFree(ts);
    return count;
}

uint32_t ffSettingsGetRpmPackageCount(FFinstance* instance)
{
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    static bool init = false;

    static LibrpmData data;

    pthread_mutex_lock(&mutex);

    if(init)
    {
        pthread_mutex_unlock(&mutex);
        return getRpmPackageCount(&data);
    }
    init = true;

    // As `data` is a static variable, it's already zero inited.
    // data.ffrpmtsCreate = NULL;

    DynamicLibrary library = FF_LIBRARY_LOAD(instance->config.librpm, "librpm.so", mutex, 0);

#define FF_LOAD_LIB_SYMBOL(fnName) data.ff ## fnName = FF_LIBRARY_LOAD_SYMBOL(library, #fnName, mutex, 0);
    FF_LOAD_LIB_SYMBOL(rpmReadConfigFiles);
    FF_LOAD_LIB_SYMBOL(rpmtsCreate);
    FF_LOAD_LIB_SYMBOL(rpmtsInitIterator);
    FF_LOAD_LIB_SYMBOL(rpmdbGetIteratorCount);
    FF_LOAD_LIB_SYMBOL(rpmdbFreeIterator);
    FF_LOAD_LIB_SYMBOL(rpmtsFree);
#undef FF_LOAD_LIB_SYMBOL

    pthread_mutex_unlock(&mutex);
    return getRpmPackageCount(&data);
}
#else //FF_HAVE_RPM
uint32_t ffSettingsGetRpmPackageCount(FFinstance* instance)
{
    FF_UNUSED(instance);
    return 0;
}
#endif //FF_HAVE_RPM

#ifdef __ANDROID__
#include <sys/system_properties.h>
void ffSettingsGetAndroidProperty(const char* propName, FFstrbuf* result) {
    ffStrbufEnsureCapacity(result, PROP_VALUE_MAX);
    result->length = __system_property_get(propName, result->chars);
}
#endif

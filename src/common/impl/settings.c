#include "fastfetch.h"
#include "common/settings.h"
#include "common/library.h"
#include "common/thread.h"
#include "common/io.h"

#include <string.h>

#ifdef FF_HAVE_GIO
    #include <gio/gio.h>

typedef struct GVariantGetters {
    FF_LIBRARY_SYMBOL(g_variant_dup_string)
    FF_LIBRARY_SYMBOL(g_variant_get_boolean)
    FF_LIBRARY_SYMBOL(g_variant_get_int32)
    FF_LIBRARY_SYMBOL(g_variant_unref)
} GVariantGetters;

static FFvariant getGVariantValue(GVariant* variant, FFvarianttype type, const GVariantGetters* variantGetters) {
    FFvariant result;

    if (variant == NULL) {
        result = FF_VARIANT_NULL;
    } else if (type == FF_VARIANT_TYPE_STRING) {
        result = (FFvariant) { .strValue = variantGetters->ffg_variant_dup_string(variant, NULL) }; // Dup string, so that variant itself can be freed
    } else if (type == FF_VARIANT_TYPE_BOOL) {
        result = (FFvariant) { .boolValue = (bool) variantGetters->ffg_variant_get_boolean(variant), .boolValueSet = true };
    } else if (type == FF_VARIANT_TYPE_INT) {
        result = (FFvariant) { .intValue = variantGetters->ffg_variant_get_int32(variant) };
    } else {
        result = FF_VARIANT_NULL;
    }

    if (variant) {
        variantGetters->ffg_variant_unref(variant);
    }

    return result;
}

typedef struct GSettingsData {
    FF_LIBRARY_SYMBOL(g_settings_schema_source_lookup)
    FF_LIBRARY_SYMBOL(g_settings_schema_has_key)
    FF_LIBRARY_SYMBOL(g_settings_new_full)
    FF_LIBRARY_SYMBOL(g_settings_get_value)
    FF_LIBRARY_SYMBOL(g_settings_get_user_value)
    FF_LIBRARY_SYMBOL(g_settings_get_default_value)
    FF_LIBRARY_SYMBOL(g_settings_schema_source_get_default)
    GSettingsSchemaSource* schemaSource;
    GVariantGetters variantGetters;

    bool inited;
} GSettingsData;

static const GSettingsData* getGSettingsData(void) {
    static GSettingsData data;

    if (!data.inited) {
        data.inited = true;
        FF_LIBRARY_LOAD(libgsettings, NULL, "libgio-2.0" FF_LIBRARY_EXTENSION, 1);
        FF_LIBRARY_LOAD_SYMBOL_VAR(libgsettings, data, g_settings_schema_source_lookup, NULL)
        FF_LIBRARY_LOAD_SYMBOL_VAR(libgsettings, data, g_settings_schema_has_key, NULL)
        FF_LIBRARY_LOAD_SYMBOL_VAR(libgsettings, data, g_settings_new_full, NULL)
        FF_LIBRARY_LOAD_SYMBOL_VAR(libgsettings, data, g_settings_get_value, NULL)
        FF_LIBRARY_LOAD_SYMBOL_VAR(libgsettings, data, g_settings_get_user_value, NULL)
        FF_LIBRARY_LOAD_SYMBOL_VAR(libgsettings, data, g_settings_get_default_value, NULL)
        FF_LIBRARY_LOAD_SYMBOL_VAR(libgsettings, data, g_settings_schema_source_get_default, NULL)

        FF_LIBRARY_LOAD_SYMBOL_VAR(libgsettings, data.variantGetters, g_variant_dup_string, NULL)
        FF_LIBRARY_LOAD_SYMBOL_VAR(libgsettings, data.variantGetters, g_variant_get_boolean, NULL)
        FF_LIBRARY_LOAD_SYMBOL_VAR(libgsettings, data.variantGetters, g_variant_get_int32, NULL)
        FF_LIBRARY_LOAD_SYMBOL_VAR(libgsettings, data.variantGetters, g_variant_unref, NULL);

        data.schemaSource = data.ffg_settings_schema_source_get_default();
        if (data.schemaSource) {
            libgsettings = NULL;
        }
    }
    if (!data.schemaSource) {
        return NULL;
    }

    return &data;
}

FFvariant ffSettingsGetGSettings(const char* schemaName, const char* path, const char* key, FFvarianttype type) {
    const GSettingsData* data = getGSettingsData();
    if (data == NULL) {
        return FF_VARIANT_NULL;
    }

    GSettingsSchema* schema = data->ffg_settings_schema_source_lookup(data->schemaSource, schemaName, true);
    if (schema == NULL) {
        return FF_VARIANT_NULL;
    }

    if (data->ffg_settings_schema_has_key(schema, key) == false) {
        return FF_VARIANT_NULL;
    }

    GSettings* settings = data->ffg_settings_new_full(schema, NULL, path);
    if (settings == NULL) {
        return FF_VARIANT_NULL;
    }

    GVariant* variant = data->ffg_settings_get_value(settings, key);
    if (variant != NULL) {
        return getGVariantValue(variant, type, &data->variantGetters);
    }

    variant = data->ffg_settings_get_user_value(settings, key);
    if (variant != NULL) {
        return getGVariantValue(variant, type, &data->variantGetters);
    }

    variant = data->ffg_settings_get_default_value(settings, key);
    return getGVariantValue(variant, type, &data->variantGetters);
}
#else  // FF_HAVE_GIO
FFvariant ffSettingsGetGSettings(const char* schemaName, const char* path, const char* key, FFvarianttype type) {
    FF_UNUSED(schemaName, path, key, type)
    return FF_VARIANT_NULL;
}
#endif // FF_HAVE_GIO

#ifdef FF_HAVE_DCONF
    #include <dconf.h>

typedef struct DConfData {
    FF_LIBRARY_SYMBOL(dconf_client_read_full)
    FF_LIBRARY_SYMBOL(dconf_client_new)
    GVariantGetters variantGetters;
    DConfClient* client;

    bool inited;
} DConfData;

static const DConfData* getDConfData(void) {
    static DConfData data;

    if (!data.inited) {
        data.inited = true;

        FF_LIBRARY_LOAD(libdconf, NULL, "libdconf" FF_LIBRARY_EXTENSION, 2);
        FF_LIBRARY_LOAD_SYMBOL_VAR(libdconf, data, dconf_client_read_full, NULL)
        FF_LIBRARY_LOAD_SYMBOL_VAR(libdconf, data, dconf_client_new, NULL)
        FF_LIBRARY_LOAD_SYMBOL_VAR(libdconf, data.variantGetters, g_variant_dup_string, NULL)
        FF_LIBRARY_LOAD_SYMBOL_VAR(libdconf, data.variantGetters, g_variant_get_boolean, NULL)
        FF_LIBRARY_LOAD_SYMBOL_VAR(libdconf, data.variantGetters, g_variant_get_int32, NULL)
        FF_LIBRARY_LOAD_SYMBOL_VAR(libdconf, data.variantGetters, g_variant_unref, NULL)

        data.client = data.ffdconf_client_new();
        if (data.client) {
            libdconf = NULL;
        }
    }
    if (!data.client) {
        return NULL;
    }

    return &data;
}

FFvariant ffSettingsGetDConf(const char* key, FFvarianttype type) {
    const DConfData* data = getDConfData();
    if (data == NULL) {
        return FF_VARIANT_NULL;
    }

    GVariant* variant = data->ffdconf_client_read_full(data->client, key, DCONF_READ_FLAGS_NONE, NULL);
    if (variant != NULL) {
        return getGVariantValue(variant, type, &data->variantGetters);
    }

    variant = data->ffdconf_client_read_full(data->client, key, DCONF_READ_USER_VALUE, NULL);
    if (variant != NULL) {
        return getGVariantValue(variant, type, &data->variantGetters);
    }

    variant = data->ffdconf_client_read_full(data->client, key, DCONF_READ_DEFAULT_VALUE, NULL);
    return getGVariantValue(variant, type, &data->variantGetters);
}
#else  // FF_HAVE_DCONF
FFvariant ffSettingsGetDConf(const char* key, FFvarianttype type) {
    FF_UNUSED(key, type)
    return FF_VARIANT_NULL;
}
#endif // FF_HAVE_DCONF

FFvariant ffSettingsGetGnome(const char* dconfKey, const char* gsettingsSchemaName, const char* gsettingsPath, const char* gsettingsKey, FFvarianttype type) {
    FFvariant gsettings = ffSettingsGetGSettings(gsettingsSchemaName, gsettingsPath, gsettingsKey, type);

    if (
        (type == FF_VARIANT_TYPE_BOOL && gsettings.boolValueSet) ||
        (type != FF_VARIANT_TYPE_BOOL && gsettings.strValue != NULL)) {
        return gsettings;
    }

    return ffSettingsGetDConf(dconfKey, type);
}

#ifdef FF_HAVE_DBUS
    #include "common/dbus.h"

FFvariant ffSettingsGetXFConf(const char* channelName, const char* propertyName, FFvarianttype type) {
    FF_DBUS_AUTO_DESTROY_DATA FFDBusData dbus = {};
    if (ffDBusLoadData(DBUS_BUS_SESSION, &dbus) != NULL) {
        return FF_VARIANT_NULL;
    }

    DBusMessage* reply = ffDBusGetMethodReply(&dbus, "org.xfce.Xfconf", "/org/xfce/Xfconf", "org.xfce.Xfconf", "GetProperty", channelName, propertyName);
    if (!reply) {
        return FF_VARIANT_NULL;
    }

    DBusMessageIter rootIterator;
    if (!dbus.lib->ffdbus_message_iter_init(reply, &rootIterator)) {
        dbus.lib->ffdbus_message_unref(reply);
        return FF_VARIANT_NULL;
    }

    if (type == FF_VARIANT_TYPE_INT) {
        int64_t value;
        if (ffDBusGetInt(&dbus, &rootIterator, &value)) {
            dbus.lib->ffdbus_message_unref(reply);
            return (FFvariant) { .intValue = (int32_t) value };
        }
        dbus.lib->ffdbus_message_unref(reply);
        return FF_VARIANT_NULL;
    }

    if (type == FF_VARIANT_TYPE_STRING) {
        FFstrbuf value = ffStrbufCreate();
        if (ffDBusGetString(&dbus, &rootIterator, &value)) {
            dbus.lib->ffdbus_message_unref(reply);
            return (FFvariant) { .strValue = value.chars }; // Leaks value.chars
        }
        dbus.lib->ffdbus_message_unref(reply);
        return FF_VARIANT_NULL;
    }

    if (type == FF_VARIANT_TYPE_BOOL) {
        bool value;
        if (ffDBusGetBool(&dbus, &rootIterator, &value)) {
            dbus.lib->ffdbus_message_unref(reply);
            return (FFvariant) { .boolValue = value, .boolValueSet = true };
        }
    }

    dbus.lib->ffdbus_message_unref(reply);
    return FF_VARIANT_NULL;
}

    #define FF_DBUS_ITER_CONTINUE(dbus, iterator)                \
        {                                                        \
            if (!(dbus).lib->ffdbus_message_iter_next(iterator)) \
                break;                                           \
            continue;                                            \
        }

FFvariant ffSettingsGetXFConfFirstMatch(const char* channelName, const char* propertyPrefix, FFvarianttype type, void* data, FFTestXfconfPropCallback* cb) {
    FF_DBUS_AUTO_DESTROY_DATA FFDBusData dbus = {};
    if (ffDBusLoadData(DBUS_BUS_SESSION, &dbus) != NULL) {
        return FF_VARIANT_NULL;
    }

    DBusMessage* reply = ffDBusGetMethodReply(&dbus, "org.xfce.Xfconf", "/org/xfce/Xfconf", "org.xfce.Xfconf", "GetAllProperties", channelName, propertyPrefix);
    if (!reply) {
        return FF_VARIANT_NULL;
    }

    DBusMessageIter rootIterator;
    if (!dbus.lib->ffdbus_message_iter_init(reply, &rootIterator)) {
        dbus.lib->ffdbus_message_unref(reply);
        return FF_VARIANT_NULL;
    }

    DBusMessageIter arrayIterator;
    dbus.lib->ffdbus_message_iter_recurse(&rootIterator, &arrayIterator);

    while (true) {
        if (dbus.lib->ffdbus_message_iter_get_arg_type(&arrayIterator) != DBUS_TYPE_DICT_ENTRY) {
            FF_DBUS_ITER_CONTINUE(dbus, &arrayIterator)
        }

        DBusMessageIter dictIterator;
        dbus.lib->ffdbus_message_iter_recurse(&arrayIterator, &dictIterator);

        const char* key;
        dbus.lib->ffdbus_message_iter_get_basic(&dictIterator, &key);

        if (cb(data, key)) {
            FF_DBUS_ITER_CONTINUE(dbus, &arrayIterator)
        }
        dbus.lib->ffdbus_message_iter_next(&dictIterator);

        if (type == FF_VARIANT_TYPE_INT) {
            int64_t value;
            if (ffDBusGetInt(&dbus, &dictIterator, &value)) {
                dbus.lib->ffdbus_message_unref(reply);
                return (FFvariant) { .intValue = (int32_t) value };
            }
            dbus.lib->ffdbus_message_unref(reply);
            return FF_VARIANT_NULL;
        }

        if (type == FF_VARIANT_TYPE_STRING) {
            FFstrbuf value = ffStrbufCreate();
            if (ffDBusGetString(&dbus, &dictIterator, &value)) {
                dbus.lib->ffdbus_message_unref(reply);
                return (FFvariant) { .strValue = value.chars }; // Leaks value.chars
            }
            dbus.lib->ffdbus_message_unref(reply);
            return FF_VARIANT_NULL;
        }

        if (type == FF_VARIANT_TYPE_BOOL) {
            bool value;
            if (ffDBusGetBool(&dbus, &dictIterator, &value)) {
                dbus.lib->ffdbus_message_unref(reply);
                return (FFvariant) { .boolValue = value, .boolValueSet = true };
            }
        }

        dbus.lib->ffdbus_message_unref(reply);
        return FF_VARIANT_NULL;
    }

    dbus.lib->ffdbus_message_unref(reply);
    return FF_VARIANT_NULL;
}
#else  // FF_HAVE_DBUS
FFvariant ffSettingsGetXFConf(const char* channelName, const char* propertyName, FFvarianttype type) {
    FF_UNUSED(channelName, propertyName, type)
    return FF_VARIANT_NULL;
}
FFvariant ffSettingsGetXFConfFirstMatch(const char* channelName, const char* propertyPrefix, FFvarianttype type, void* data, FFTestXfconfPropCallback* cb) {
    FF_UNUSED(channelName, propertyPrefix, type, data, cb);
    return FF_VARIANT_NULL;
}
#endif // FF_HAVE_DBUS

#ifdef FF_HAVE_SQLITE3
    #include <sqlite3.h>

typedef struct SQLiteData {
    FF_LIBRARY_SYMBOL(sqlite3_open_v2)
    FF_LIBRARY_SYMBOL(sqlite3_prepare_v2)
    FF_LIBRARY_SYMBOL(sqlite3_step)
    FF_LIBRARY_SYMBOL(sqlite3_data_count)
    FF_LIBRARY_SYMBOL(sqlite3_column_int)
    FF_LIBRARY_SYMBOL(sqlite3_column_text)
    FF_LIBRARY_SYMBOL(sqlite3_finalize)
    FF_LIBRARY_SYMBOL(sqlite3_close)

    bool inited;
} SQLiteData;

static const SQLiteData* getSQLiteData(void) {
    static SQLiteData data;

    if (!data.inited) {
        data.inited = true;
        FF_LIBRARY_LOAD(libsqlite, NULL, "libsqlite3" FF_LIBRARY_EXTENSION, 1);
        FF_LIBRARY_LOAD_SYMBOL_VAR(libsqlite, data, sqlite3_open_v2, NULL)
        FF_LIBRARY_LOAD_SYMBOL_VAR(libsqlite, data, sqlite3_prepare_v2, NULL)
        FF_LIBRARY_LOAD_SYMBOL_VAR(libsqlite, data, sqlite3_step, NULL)
        FF_LIBRARY_LOAD_SYMBOL_VAR(libsqlite, data, sqlite3_data_count, NULL)
        FF_LIBRARY_LOAD_SYMBOL_VAR(libsqlite, data, sqlite3_column_int, NULL)
        FF_LIBRARY_LOAD_SYMBOL_VAR(libsqlite, data, sqlite3_column_text, NULL)
        FF_LIBRARY_LOAD_SYMBOL_VAR(libsqlite, data, sqlite3_finalize, NULL)
        FF_LIBRARY_LOAD_SYMBOL_VAR(libsqlite, data, sqlite3_close, NULL)
        libsqlite = NULL;
    }

    if (!data.ffsqlite3_close) {
        return NULL;
    }

    return &data;
}

int ffSettingsGetSQLite3Int(const char* dbPath, const char* query) {
    if (!ffPathExists(dbPath, FF_PATHTYPE_FILE)) {
        return 0;
    }

    const SQLiteData* data = getSQLiteData();
    if (data == NULL) {
        return 0;
    }

    sqlite3* db;
    if (data->ffsqlite3_open_v2(dbPath, &db, SQLITE_OPEN_READONLY, NULL) != SQLITE_OK) {
        return 0;
    }

    sqlite3_stmt* stmt;
    if (data->ffsqlite3_prepare_v2(db, query, (int) strlen(query), &stmt, NULL) != SQLITE_OK) {
        data->ffsqlite3_close(db);
        return 0;
    }

    if (data->ffsqlite3_step(stmt) != SQLITE_ROW || data->ffsqlite3_data_count(stmt) < 1) {
        data->ffsqlite3_finalize(stmt);
        data->ffsqlite3_close(db);
        return 0;
    }

    int result = data->ffsqlite3_column_int(stmt, 0);

    data->ffsqlite3_finalize(stmt);
    data->ffsqlite3_close(db);

    return result;
}

bool ffSettingsGetSQLite3String(const char* dbPath, const char* query, FFstrbuf* result) {
    if (!ffPathExists(dbPath, FF_PATHTYPE_FILE)) {
        return false;
    }

    const SQLiteData* data = getSQLiteData();
    if (data == NULL) {
        return false;
    }

    sqlite3* db;
    if (data->ffsqlite3_open_v2(dbPath, &db, SQLITE_OPEN_READONLY, NULL) != SQLITE_OK) {
        return false;
    }

    sqlite3_stmt* stmt;
    if (data->ffsqlite3_prepare_v2(db, query, (int) strlen(query), &stmt, NULL) != SQLITE_OK) {
        data->ffsqlite3_close(db);
        return false;
    }

    if (data->ffsqlite3_step(stmt) != SQLITE_ROW || data->ffsqlite3_data_count(stmt) < 1) {
        data->ffsqlite3_finalize(stmt);
        data->ffsqlite3_close(db);
        return false;
    }

    ffStrbufSetS(result, (const char*) data->ffsqlite3_column_text(stmt, 0));

    data->ffsqlite3_finalize(stmt);
    data->ffsqlite3_close(db);

    return true;
}
#else  // FF_HAVE_SQLITE3
int ffSettingsGetSQLite3Int(const char* dbPath, const char* query) {
    FF_UNUSED(dbPath, query)
    return 0;
}
bool ffSettingsGetSQLite3String(const char* dbPath, const char* query, FFstrbuf* result) {
    FF_UNUSED(dbPath, query, result)
    return false;
}
#endif // FF_HAVE_SQLITE3

#ifdef __ANDROID__
    #include <sys/system_properties.h>
bool ffSettingsGetAndroidProperty(const char* propName, FFstrbuf* result) {
    ffStrbufEnsureFree(result, PROP_VALUE_MAX);
    int len = __system_property_get(propName, result->chars + result->length);
    if (len <= 0) {
        return false;
    }
    result->length += (uint32_t) len;
    result->chars[result->length] = '\0';
    return true;
}
#elif defined(__FreeBSD__)
    #include <kenv.h>
bool ffSettingsGetFreeBSDKenv(const char* propName, FFstrbuf* result) {
    // https://wiki.ghostbsd.org/index.php/Kenv
    ffStrbufEnsureFree(result, KENV_MVALLEN);
    int len = kenv(KENV_GET, propName, result->chars + result->length, KENV_MVALLEN);
    if (len <= 1) {
        return false; // number of bytes copied, including NUL terminator
    }
    result->length += (uint32_t) len - 1;
    return true;
}
#endif

#ifdef FF_HAVE_EET
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wconversion"
    #pragma GCC diagnostic ignored "-Wsign-conversion"
    #pragma GCC diagnostic ignored "-Wfloat-conversion"
    #include <Eet.h>
    #pragma GCC diagnostic pop

typedef struct E_Font_Default {
    char* text_class;
    char* font;
    int size;
} E_Font_Default;

typedef struct E_Config {
    char* theme_default_border_style;
    char* icon_theme;
    int use_e_cursor;
    int cursor_size;
    char* desktop_default_background;
    Eina_List* font_defaults;
} E_Config; // Must be the same name as the top level struct in e.cfg

    #define FF_EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(clas, type) \
        (ffeet_eina_file_data_descriptor_class_set(clas, sizeof(*(clas)), #type, sizeof(type)))
    #define FF_EET_DATA_DESCRIPTOR_ADD_BASIC(edd, struct_type, member, type)                                                                                 \
        do {                                                                                                                                                 \
            struct_type ___ett;                                                                                                                              \
            ffeet_data_descriptor_element_add(edd, #member, type, EET_G_UNKNOWN, (char*) (&(___ett.member)) - (char*) (&(___ett)), 0, /* 0,  */ NULL, NULL); \
        } while (0)
    #define FF_EET_DATA_DESCRIPTOR_ADD_LIST(edd, struct_type, member, subtype)                                                                                       \
        do {                                                                                                                                                         \
            struct_type ___ett;                                                                                                                                      \
            ffeet_data_descriptor_element_add(edd, #member, EET_T_UNKNOW, EET_G_LIST, (char*) (&(___ett.member)) - (char*) (&(___ett)), 0, /* 0,  */ NULL, subtype); \
        } while (0)

bool ffSettingsGetEnlightenmentProperty(ffEnlightenmentSettings* result) {
    FF_LIBRARY_LOAD(libeet, false, "libeet" FF_LIBRARY_EXTENSION, 1);
    FF_LIBRARY_LOAD_SYMBOL(libeet, eet_init, false);
    FF_LIBRARY_LOAD_SYMBOL(libeet, eet_open, false);
    FF_LIBRARY_LOAD_SYMBOL(libeet, eet_data_descriptor_file_new, false);
    FF_LIBRARY_LOAD_SYMBOL(libeet, eet_data_read, false);
    FF_LIBRARY_LOAD_SYMBOL(libeet, eet_close, false);
    FF_LIBRARY_LOAD_SYMBOL(libeet, eet_shutdown, false);
    FF_LIBRARY_LOAD_SYMBOL(libeet, eet_data_descriptor_free, false);
    FF_LIBRARY_LOAD_SYMBOL(libeet, eet_eina_file_data_descriptor_class_set, false);
    FF_LIBRARY_LOAD_SYMBOL(libeet, eet_data_descriptor_element_add, false);

    if (ffeet_init() == 0) {
        return false;
    }

    FF_STRBUF_AUTO_DESTROY fileName = ffStrbufCreateCopy(&instance.state.platform.homeDir);
    ffStrbufAppendS(&fileName, ".e/e/config/standard/e.cfg");

    Eet_File* ef = ffeet_open(fileName.chars, EET_FILE_MODE_READ);
    if (!ef) {
        ffeet_shutdown();
        return false;
    }

    Eet_Data_Descriptor_Class fontDdc;
    FF_EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&fontDdc, E_Font_Default);
    Eet_Data_Descriptor* fontDdd = ffeet_data_descriptor_file_new(&fontDdc);
    if (!fontDdd) {
        ffeet_close(ef);
        ffeet_shutdown();
        return false;
    }
    FF_EET_DATA_DESCRIPTOR_ADD_BASIC(fontDdd, E_Font_Default, text_class, EET_T_STRING);
    FF_EET_DATA_DESCRIPTOR_ADD_BASIC(fontDdd, E_Font_Default, font, EET_T_STRING);
    FF_EET_DATA_DESCRIPTOR_ADD_BASIC(fontDdd, E_Font_Default, size, EET_T_INT);

    Eet_Data_Descriptor_Class eddc;
    FF_EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, E_Config);
    Eet_Data_Descriptor* edd = ffeet_data_descriptor_file_new(&eddc);
    if (!edd) {
        ffeet_data_descriptor_free(fontDdd);
        ffeet_close(ef);
        ffeet_shutdown();
        return false;
    }

    FF_EET_DATA_DESCRIPTOR_ADD_BASIC(edd, E_Config, theme_default_border_style, EET_T_STRING);
    FF_EET_DATA_DESCRIPTOR_ADD_BASIC(edd, E_Config, icon_theme, EET_T_STRING);
    FF_EET_DATA_DESCRIPTOR_ADD_BASIC(edd, E_Config, use_e_cursor, EET_T_INT);
    FF_EET_DATA_DESCRIPTOR_ADD_BASIC(edd, E_Config, cursor_size, EET_T_INT);
    FF_EET_DATA_DESCRIPTOR_ADD_BASIC(edd, E_Config, desktop_default_background, EET_T_STRING);
    FF_EET_DATA_DESCRIPTOR_ADD_LIST(edd, E_Config, font_defaults, fontDdd);

    E_Config* parsed = ffeet_data_read(ef, edd, "config");

    if (parsed) {
        // TODO: find a better method to get the main theme name
        result->theme = parsed->theme_default_border_style;
        result->icon_theme = parsed->icon_theme;
        result->use_e_cursor = !!parsed->use_e_cursor;
        result->cursor_size = parsed->cursor_size;
        result->desktop_default_background = parsed->desktop_default_background;

        E_Font_Default* firstFont = eina_list_data_get(parsed->font_defaults);
        if (firstFont) {
            result->font = firstFont->font;
        }
    }

    ffeet_close(ef);
    ffeet_data_descriptor_free(edd);
    ffeet_data_descriptor_free(fontDdd);
    if (!parsed) {
        // We don't shutdown eet so that `result->*` are not freed
        ffeet_shutdown();
    }

    return !!parsed;
}
#else
bool ffSettingsGetEnlightenmentProperty(FF_A_UNUSED ffEnlightenmentSettings* result) {
    return false;
}
#endif

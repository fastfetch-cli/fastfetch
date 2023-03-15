#include "common/json.h"

#ifdef FF_HAVE_JSONC

#include "common/library.h"
#include "common/thread.h"

#include <assert.h>

typedef struct FFJsonLibrary
{
    FF_LIBRARY_SYMBOL(json_tokener_parse)
    FF_LIBRARY_SYMBOL(json_object_is_type)
    FF_LIBRARY_SYMBOL(json_object_get_array)
    FF_LIBRARY_SYMBOL(json_object_get_boolean)
    FF_LIBRARY_SYMBOL(json_object_get_double)
    FF_LIBRARY_SYMBOL(json_object_get_int)
    FF_LIBRARY_SYMBOL(json_object_get_string_len)
    FF_LIBRARY_SYMBOL(json_object_get_string)
    FF_LIBRARY_SYMBOL(json_object_get_object)
    FF_LIBRARY_SYMBOL(json_object_object_del)
    FF_LIBRARY_SYMBOL(json_object_object_get)
    FF_LIBRARY_SYMBOL(json_object_object_length)
    FF_LIBRARY_SYMBOL(json_object_put)
} FFJsonLibrary;

static const FFJsonLibrary* loadLibSymbols(const FFinstance* instance)
{
    assert(instance);

    static FFJsonLibrary lib;
    FF_LIBRARY_LOAD(libjsonc, &instance->config.libJSONC, NULL,
        #ifdef _WIN32
            "libjson-c-5" FF_LIBRARY_EXTENSION, -1
        #else
            "libjson-c" FF_LIBRARY_EXTENSION, 5
        #endif
    )
    FF_LIBRARY_LOAD_SYMBOL_VAR(libjsonc, lib, json_tokener_parse, NULL)
    FF_LIBRARY_LOAD_SYMBOL_VAR(libjsonc, lib, json_object_is_type, NULL)
    FF_LIBRARY_LOAD_SYMBOL_VAR(libjsonc, lib, json_object_get_array, NULL)
    FF_LIBRARY_LOAD_SYMBOL_VAR(libjsonc, lib, json_object_get_boolean, NULL)
    FF_LIBRARY_LOAD_SYMBOL_VAR(libjsonc, lib, json_object_get_double, NULL)
    FF_LIBRARY_LOAD_SYMBOL_VAR(libjsonc, lib, json_object_get_int, NULL)
    FF_LIBRARY_LOAD_SYMBOL_VAR(libjsonc, lib, json_object_get_string_len, NULL)
    FF_LIBRARY_LOAD_SYMBOL_VAR(libjsonc, lib, json_object_get_string, NULL)
    FF_LIBRARY_LOAD_SYMBOL_VAR(libjsonc, lib, json_object_get_object, NULL)
    FF_LIBRARY_LOAD_SYMBOL_VAR(libjsonc, lib, json_object_object_del, NULL)
    FF_LIBRARY_LOAD_SYMBOL_VAR(libjsonc, lib, json_object_object_get, NULL)
    FF_LIBRARY_LOAD_SYMBOL_VAR(libjsonc, lib, json_object_object_length, NULL)
    FF_LIBRARY_LOAD_SYMBOL_VAR(libjsonc, lib, json_object_put, NULL)
    libjsonc = NULL; // don't dlclose automatically
    return &lib;
}

const FFJsonLibrary* ffJsonLib = NULL;
bool ffJsonLoadLibrary(const FFinstance* instance)
{
    static bool loaded = false;
    static FFThreadMutex mutex = FF_THREAD_MUTEX_INITIALIZER;

    if (!loaded)
    {
        ffThreadMutexLock(&mutex);

        if(!loaded)
        {
            loaded = true;
            ffJsonLib = loadLibSymbols(instance);
        }

        ffThreadMutexUnlock(&mutex);
    }
    return !!ffJsonLib;
}

struct json_object *json_tokener_parse(const char *str)
{
    assert(ffJsonLib);
    return ffJsonLib->ffjson_tokener_parse(str);
}

int json_object_is_type(const json_object *obj, enum json_type type)
{
    assert(ffJsonLib);
    return ffJsonLib->ffjson_object_is_type(obj, type);
}

struct array_list *json_object_get_array(const json_object *obj)
{
    assert(ffJsonLib);
    return ffJsonLib->ffjson_object_get_array(obj);
}

json_bool json_object_get_boolean(const json_object *obj)
{
    assert(ffJsonLib);
    return ffJsonLib->ffjson_object_get_boolean(obj);
}

double json_object_get_double(const json_object *obj)
{
    assert(ffJsonLib);
    return ffJsonLib->ffjson_object_get_double(obj);
}

int32_t json_object_get_int(const json_object *obj)
{
    assert(ffJsonLib);
    return ffJsonLib->ffjson_object_get_int(obj);
}

int json_object_get_string_len(const json_object *obj)
{
    assert(ffJsonLib);
    return ffJsonLib->ffjson_object_get_string_len(obj);
}

const char *json_object_get_string(json_object *obj)
{
    assert(ffJsonLib);
    return ffJsonLib->ffjson_object_get_string(obj);
}

struct lh_table *json_object_get_object(const json_object *obj)
{
    assert(ffJsonLib);
    return ffJsonLib->ffjson_object_get_object(obj);
}

void json_object_object_del(struct json_object *obj, const char *key)
{
    assert(ffJsonLib);
    return ffJsonLib->ffjson_object_object_del(obj, key);
}

struct json_object *json_object_object_get(const json_object *obj, const char *key)
{
    assert(ffJsonLib);
    return ffJsonLib->ffjson_object_object_get(obj, key);
}

int json_object_object_length(const json_object *obj)
{
    assert(ffJsonLib);
    return ffJsonLib->ffjson_object_object_length(obj);
}

int json_object_put(json_object *obj)
{
    assert(ffJsonLib);
    return ffJsonLib->ffjson_object_put(obj);
}

#endif

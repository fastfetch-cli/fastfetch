#pragma once

#ifdef FF_HAVE_JSONC

#include "fastfetch.h"
#include "common/library.h"

#include <assert.h>
#include <json-c/json.h>

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

extern const FFJsonLibrary* ffJsonLib;

bool ffJsonLoadLibrary(const FFinstance* instance);

static inline void wrapJsoncFree(json_object** root)
{
    assert(root);
    if (*root)
        json_object_put(*root);
}

extern inline __attribute__((__gnu_inline__, __always_inline__))
struct json_object *json_tokener_parse(const char *str)
{
    assert(ffJsonLib);
    return ffJsonLib->ffjson_tokener_parse(str);
}

extern inline __attribute__((__gnu_inline__, __always_inline__))
int json_object_is_type(const json_object *obj, enum json_type type)
{
    assert(ffJsonLib);
    return ffJsonLib->ffjson_object_is_type(obj, type);
}

extern inline __attribute__((__gnu_inline__, __always_inline__))
struct array_list *json_object_get_array(const json_object *obj)
{
    assert(ffJsonLib);
    return ffJsonLib->ffjson_object_get_array(obj);
}

extern inline __attribute__((__gnu_inline__, __always_inline__))
json_bool json_object_get_boolean(const json_object *obj)
{
    assert(ffJsonLib);
    return ffJsonLib->ffjson_object_get_boolean(obj);
}

extern inline __attribute__((__gnu_inline__, __always_inline__))
double json_object_get_double(const json_object *obj)
{
    assert(ffJsonLib);
    return ffJsonLib->ffjson_object_get_double(obj);
}

extern inline __attribute__((__gnu_inline__, __always_inline__))
int32_t json_object_get_int(const json_object *obj)
{
    assert(ffJsonLib);
    return ffJsonLib->ffjson_object_get_int(obj);
}

extern inline __attribute__((__gnu_inline__, __always_inline__))
int json_object_get_string_len(const json_object *obj)
{
    assert(ffJsonLib);
    return ffJsonLib->ffjson_object_get_string_len(obj);
}

extern inline __attribute__((__gnu_inline__, __always_inline__))
const char *json_object_get_string(json_object *obj)
{
    assert(ffJsonLib);
    return ffJsonLib->ffjson_object_get_string(obj);
}

extern inline __attribute__((__gnu_inline__, __always_inline__))
struct lh_table *json_object_get_object(const json_object *obj)
{
    assert(ffJsonLib);
    return ffJsonLib->ffjson_object_get_object(obj);
}

extern inline __attribute__((__gnu_inline__, __always_inline__))
void json_object_object_del(struct json_object *obj, const char *key)
{
    assert(ffJsonLib);
    return ffJsonLib->ffjson_object_object_del(obj, key);
}

extern inline __attribute__((__gnu_inline__, __always_inline__))
struct json_object *json_object_object_get(const json_object *obj, const char *key)
{
    assert(ffJsonLib);
    return ffJsonLib->ffjson_object_object_get(obj, key);
}

extern inline __attribute__((__gnu_inline__, __always_inline__))
int json_object_object_length(const json_object *obj)
{
    assert(ffJsonLib);
    return ffJsonLib->ffjson_object_object_length(obj);
}

extern inline __attribute__((__gnu_inline__, __always_inline__))
int json_object_put(json_object *obj)
{
    assert(ffJsonLib);
    return ffJsonLib->ffjson_object_put(obj);
}

#endif

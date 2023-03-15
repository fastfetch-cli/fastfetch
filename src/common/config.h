#pragma once

#ifdef FF_HAVE_JSONC

#include <json-c/json.h>

#include "common/library.h"

typedef struct JSONCData
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
    FF_LIBRARY_SYMBOL(json_object_object_get)
    FF_LIBRARY_SYMBOL(json_object_put)
} JSONCData;

bool ffJsonConfigParseModuleArgs(JSONCData* data, const char* key, json_object* val, FFModuleArgs* moduleArgs);
const char* ffJsonConfigParseEnum(JSONCData* data, json_object* val, int* result, FFKeyValuePair pairs[]);

// Modified from json_object_object_foreach
#define FF_JSON_OBJECT_OBJECT_FOREACH(data, obj, keyVar, valVar)                                  \
    const char* keyVar = NULL;                                                                               \
    json_object* valVar = NULL;                                                                              \
    for (struct lh_entry* entry##keyVar = (obj) ? lh_table_head(data->ffjson_object_get_object(obj)) : NULL; \
         ({                                                                                                  \
             if (entry##keyVar)                                                                              \
             {                                                                                               \
                 keyVar = (const char*) lh_entry_k(entry##keyVar);                                           \
                 valVar = (json_object*) lh_entry_v(entry##keyVar);                                          \
             };                                                                                              \
             entry##keyVar;                                                                                  \
         });                                                                                                 \
         entry##keyVar = lh_entry_next(entry##keyVar))

#endif
